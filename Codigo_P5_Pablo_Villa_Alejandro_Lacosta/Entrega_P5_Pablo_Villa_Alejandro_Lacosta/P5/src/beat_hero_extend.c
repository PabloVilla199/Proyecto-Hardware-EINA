/******************************************************************************
 * Fichero: beat_hero_extend.c
 * Proyecto: Guitar Hero - Prácticas P.H. 2025
 *
 * Descripción:
 * Implementación FINAL v3 del juego "Beat Hero".
 * - Filtro anti-rebotes.
 * - Modo suspensión (Sleep) al finalizar (despierta solo con 3/4).
 * - Salida inmediata al menú (sin delay de 2s).
 * - Logs detallados de Estado/Acierto/Fallo controlados por DEBUG.
 *
 *****************************************************************************/
 
#include "beat_hero.h"
#include "drv_leds.h"
#include "drv_botones.h"
#include "drv_tiempo.h"
#include "drv_consumo.h" 
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "hal_random.h"
#include "rt_GE.h"
#include <stddef.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#include "drv_uart.h"
#include <stdio.h>
#endif

// ============================================================================
// DEFINICIONES Y CONSTANTES
// ============================================================================

#define NUM_COMPASES        15
#define BPM_INICIAL         50
#define COMPAS_MS           (60000 / BPM_INICIAL)
#define VENTANA_ACIERTO_MS  (COMPAS_MS * 0.4)
#define VENTANA_PERFECTO_MS (VENTANA_ACIERTO_MS * 0.2)
#define TIEMPO_ENTRE_COMPASES 150
#define PUNTUACION_EXITO    20
#define PUNTUACION_FALLO    -8
#define TIEMPO_TRANSICION   80
#define TIEMPO_EXTENDIDO_MULTIPLICADOR 1.5

// IDs de timeouts
#define ID_TIMEOUT_COMPAS   0xFE
#define ID_TIMEOUT_INACTIVO 0xFD
#define ID_TIMEOUT_FIN      0xFC
#define ID_TRANSICION       0xFB
#define ID_LONGPRESS        0xFA

// Mapping LEDs
#define LED_1   ((LED_id_t)1)
#define LED_2   ((LED_id_t)2)
#define LED_3   ((LED_id_t)3)
#define LED_4   ((LED_id_t)4)

// Patrones
#define PATRON_LED_1        0x01
#define PATRON_LED_2        0x02
#define PATRON_AMBOS        0x03
#define PATRON_NINGUNO      0x00

// Botones
#define BOTON_1             0
#define BOTON_2             1
#define BOTON_3             2
#define BOTON_4             3



#if DEBUG
static char buffer[128];
#define LOG_MSG(msg) do { sprintf(buffer, "[GAME] %s\r\n", msg); drv_uart_send(buffer); } while(0)
#define LOG_VAR(label, val) do { sprintf(buffer, "[GAME] %s: %d\r\n", label, val); drv_uart_send(buffer); } while(0)
#define LOG_STATE(st) do { sprintf(buffer, "[FSM] Cambio a Estado: %d\r\n", st); drv_uart_send(buffer); } while(0)
#else
#define LOG_MSG(msg)
#define LOG_VAR(label, val)
#define LOG_STATE(st)
#endif

// ============================================================================
// ESTRUCTURA DE ESTADÍSTICAS
// ============================================================================
#if DEBUG
typedef struct {
    uint8_t compases_acertados;
    uint8_t compases_fallados;
    uint8_t compases_perfectos;
    uint8_t compases_sin_respuesta;
    uint8_t pulsaciones_correctas;
    uint8_t pulsaciones_incorrectas;  
    uint8_t total_compases_activos; 
    uint8_t aciertos_activos;       
    bool compas_actual_acertado;
    bool compas_actual_perfecto;
} estadisticas_t;

static estadisticas_t stats;
#endif

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

static uint8_t compas[3];
static uint8_t compases_restantes;
static uint8_t compas_actual;
static estado_juego_t estado_actual;
static int32_t puntuacion;
static uint32_t tiempo_inicio_compas;
static uint8_t nivel;
static bool entrada_valida;
static uint8_t patron_esperado_actual;
static bool esperando_reinicio;
static uint8_t etapa_secuencia_fin;
static bool en_transicion;
static uint8_t paso_inicio;

static bool longpress_en_curso = false;
static int boton_longpress_activo = -1;

static const uint8_t FIN_MASK[] = {
    0x05, 0x00, 0x05, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0F, 0x00, 0x0F
};

// ============================================================================
// PROTOTIPOS
// ============================================================================

static void juego_fsm(EVENTO_T ev, uint32_t aux);
static void procesar_estado_init(bool es_timeout_inactivo, bool es_boton);
static void procesar_estado_show_sequence(bool es_timeout_compas, bool es_transicion,
                                          bool es_boton_salida, bool es_boton);
static void procesar_estado_wait_for_input(bool es_boton_salida, bool es_boton,
                                           bool es_timeout_compas, uint32_t aux);
static void procesar_estado_fin_partida(bool es_timeout_inactivo, bool es_timeout_fin,
                                        bool es_boton_salida, bool es_boton, uint32_t aux);

static void generar_nuevo_compas(void);
static void avanzar_compas(void);
static bool verificar_fin_juego(void);
static void aumentar_dificultad_si_corresponde(void);

static void evaluar_pulsacion(uint8_t boton_pulsado, uint32_t tiempo_reaccion);
static bool es_pulsacion_correcta(uint8_t boton_pulsado);
static int calcular_puntos_por_timing(uint32_t tiempo_reaccion);
static void procesar_acierto(int puntos, uint32_t tiempo_reaccion);
static void procesar_fallo(void);
static void evaluar_compas_sin_entrada(void);

static void mostrar_transicion(void);
static void mostrar_patron_final(void);
static void apagar_todos_leds(void);
static void configurar_leds_patron(uint8_t patron_top, uint8_t patron_bottom);
static void configurar_leds_por_mask(uint8_t mask);

static void iniciar_secuencia_fin(void);
static void avanzar_secuencia_fin(void);
static void configurar_leds_etapa_fin(uint8_t etapa);
static void iniciar_secuencia_inicio(void);

static void reiniciar_juego(void);
static void inicializar_drivers(void);
static void inicializar_compases(void);

#if DEBUG
static void inicializar_estadisticas(void);
static void resetear_estadisticas_compas_actual(void);
static void actualizar_estadisticas_compas(void);
static void mostrar_estadisticas_finales(void);
static float calcular_porcentaje(uint8_t valor, uint8_t total);
static const char* evaluar_rendimiento(float porcentaje_aciertos);
#endif

// ============================================================================
// INICIO Y CONFIGURACIÓN
// ============================================================================

void beat_hero_extend_iniciar(void) {
    LOG_MSG("=== BEAT HERO INICIADO ===");
    
    rt_GE_iniciar(10);
    inicializar_drivers();
    reiniciar_juego();
    
    svc_GE_suscribir(ev_PULSAR_BOTON, 2, juego_fsm);
    svc_GE_suscribir(ev_BOTON_RETARDO, 2, juego_fsm);

    iniciar_secuencia_inicio();
    rt_GE_lanzador();
}

static void inicializar_drivers(void) {
    drv_leds_iniciar();
    drv_botones_iniciar(NULL, ev_PULSAR_BOTON, ev_PULSAR_BOTON);
    hal_random_iniciar(drv_tiempo_actual_ms());
}

static void reiniciar_juego(void) {
    LOG_MSG("Reiniciando variables de juego...");

    estado_actual = e_INIT;
    puntuacion = 0;
    nivel = 1;
    compases_restantes = NUM_COMPASES;
    compas_actual = 0;
    esperando_reinicio = false;
    etapa_secuencia_fin = 0;
    en_transicion = false;
    paso_inicio = 0;

    #if DEBUG
    inicializar_estadisticas();
    #else
    entrada_valida = false;
    #endif

    inicializar_compases();
    apagar_todos_leds();
}

static void inicializar_compases(void) {
    for(int i = 0; i < 3; i++) compas[i] = 0;
    for(int i = 0; i < 3; i++) generar_nuevo_compas();
}

static void iniciar_secuencia_inicio(void) {
    paso_inicio = 0;
    svc_alarma_activar(svc_alarma_codificar(false, 300, 0), 
                       ev_PULSAR_BOTON, ID_TIMEOUT_INACTIVO);
}

// ============================================================================
// MÁQUINA DE ESTADOS (FSM)
// ============================================================================

static void juego_fsm(EVENTO_T ev, uint32_t aux) {
    const bool es_evento_boton = (ev == ev_PULSAR_BOTON);
    const bool es_longpress_timer = (ev == ev_BOTON_RETARDO && aux == ID_LONGPRESS);

    const bool es_timeout_compas = (es_evento_boton && aux == ID_TIMEOUT_COMPAS);
    const bool es_timeout_inactivo = (es_evento_boton && aux == ID_TIMEOUT_INACTIVO);
    const bool es_timeout_fin = (es_evento_boton && aux == ID_TIMEOUT_FIN);
    const bool es_transicion = (es_evento_boton && aux == ID_TRANSICION);
    const bool es_boton = (es_evento_boton && aux < 4);
    const bool es_boton_salida = (es_boton && (aux == BOTON_3 || aux == BOTON_4));

    #if DEBUG
    // Descomentar si se quiere loguear CADA evento
    // sprintf(buffer, "[FSM] Estado: %d, Evento: 0x%lX\r\n", estado_actual, aux); drv_uart_send(buffer);
    #endif

    if (es_longpress_timer) {
        longpress_en_curso = false;
        int btn = boton_longpress_activo;
        boton_longpress_activo = -1;

        if (btn >= 0 && drv_boton_esta_pulsado((uint8_t)btn)) {
            LOG_MSG("Reinicio forzado por Long-Press!");
            reiniciar_juego();
            iniciar_secuencia_inicio();
        }
        return;
    }

    if (es_boton && (aux == BOTON_3 || aux == BOTON_4) && !longpress_en_curso && !esperando_reinicio) {
        longpress_en_curso = true;
        boton_longpress_activo = (int)aux;
        svc_alarma_activar(svc_alarma_codificar(false, 3000, 0), ev_BOTON_RETARDO, ID_LONGPRESS);
    }
    // -------------------------------------------------

    switch (estado_actual) {
        case e_INIT:
            procesar_estado_init(es_timeout_inactivo, es_boton);
            break;

        case e_SHOW_SEQUENCE:
            procesar_estado_show_sequence(es_timeout_compas, es_transicion,
                                          es_boton_salida, es_boton);
            break;

        case e_WAIT_FOR_INPUT:
            procesar_estado_wait_for_input(es_boton_salida, es_boton,
                                           es_timeout_compas, aux);
            break;

        case e_FIN_PARTIDA:
            procesar_estado_fin_partida(es_timeout_inactivo, es_timeout_fin,
                                        es_boton_salida, es_boton, aux);
            break;
    }
}

static void procesar_estado_init(bool es_timeout_inactivo, bool es_boton) {
    if (es_timeout_inactivo) {
        if (paso_inicio < 4) {
            apagar_todos_leds();
            if (paso_inicio % 2 == 0) {
                drv_led_establecer(LED_1, LED_ON);
                drv_led_establecer(LED_3, LED_ON);
            } else {
                drv_led_establecer(LED_2, LED_ON);
                drv_led_establecer(LED_4, LED_ON);
            }
            paso_inicio++;
            svc_alarma_activar(svc_alarma_codificar(false, 250, 0),
                              ev_PULSAR_BOTON, ID_TIMEOUT_INACTIVO);
        } else {
            apagar_todos_leds();
            LOG_MSG(">>> JUEGO COMENZADO <<<");
            estado_actual = e_SHOW_SEQUENCE;
            LOG_STATE(e_SHOW_SEQUENCE);
            compas_actual = 0;
            svc_alarma_activar(svc_alarma_codificar(false, 400, 0),
                              ev_PULSAR_BOTON, ID_TIMEOUT_COMPAS);
        }
    }
}

static void procesar_estado_show_sequence(bool es_timeout_compas, bool es_transicion,
                                          bool es_boton_salida, bool es_boton) {
    if (es_timeout_compas && !en_transicion) {
        en_transicion = true;
        mostrar_transicion();
        svc_alarma_activar(svc_alarma_codificar(false, TIEMPO_TRANSICION, 0),
                          ev_PULSAR_BOTON, ID_TRANSICION);
    }
    else if (es_transicion && en_transicion) {
        mostrar_patron_final();
        en_transicion = false;

        if (compas_actual >= 1) {
            patron_esperado_actual = compas[0];
            #if DEBUG
            resetear_estadisticas_compas_actual();
            #else
            entrada_valida = false;
            #endif
        } else {
            patron_esperado_actual = 0;
        }

        tiempo_inicio_compas = drv_tiempo_actual_ms();
        estado_actual = e_WAIT_FOR_INPUT;
        LOG_STATE(e_WAIT_FOR_INPUT);

        uint32_t tiempo_compas = COMPAS_MS;
        if (compases_restantes <= 3) {
            tiempo_compas = COMPAS_MS * TIEMPO_EXTENDIDO_MULTIPLICADOR;
        }

        svc_alarma_activar(svc_alarma_codificar(false, tiempo_compas, 0),
                          ev_PULSAR_BOTON, ID_TIMEOUT_COMPAS);
    }
    else if (es_boton_salida) {
        LOG_MSG("Usuario pulso SALIR (Btn 3/4)");
        puntuacion = PUNTUACION_FALLO - 1;
        iniciar_secuencia_fin();
    }
}

static void procesar_estado_wait_for_input(bool es_boton_salida, bool es_boton,
                                           bool es_timeout_compas, uint32_t aux) {
    if (es_boton_salida) {
        LOG_MSG("Usuario pulso SALIR durante juego");
        puntuacion = PUNTUACION_FALLO - 1;
        iniciar_secuencia_fin();
    }
    else if (es_boton && compas_actual >= 1) {
        evaluar_pulsacion((uint8_t)aux, drv_tiempo_actual_ms() - tiempo_inicio_compas);
    }
    else if (es_timeout_compas) {
        if (compas_actual >= 1) {
            evaluar_compas_sin_entrada();
            #if DEBUG
            actualizar_estadisticas_compas();
            #endif
        }

        avanzar_compas();
        aumentar_dificultad_si_corresponde();

        if (verificar_fin_juego()) {
            estado_actual = e_FIN_PARTIDA;
            LOG_STATE(e_FIN_PARTIDA);
            iniciar_secuencia_fin();
        } else {
            estado_actual = e_SHOW_SEQUENCE;
            LOG_STATE(e_SHOW_SEQUENCE);
            svc_alarma_activar(svc_alarma_codificar(false, TIEMPO_ENTRE_COMPASES, 0),
                              ev_PULSAR_BOTON, ID_TIMEOUT_COMPAS);
        }
    }
}

static void procesar_estado_fin_partida(bool es_timeout_inactivo, bool es_timeout_fin,
                                        bool es_boton_salida, bool es_boton, uint32_t aux) {
    
    // --- FASE ANIMACION ---
    if (!esperando_reinicio) {
        if (es_timeout_inactivo) {
            avanzar_secuencia_fin();
        }
        else if (es_timeout_fin) {
            LOG_MSG("Secuencia fin completada -> Durmiendo");
            apagar_todos_leds();
            esperando_reinicio = true;

            #if DEBUG
            mostrar_estadisticas_finales();
            #endif
            LOG_MSG("Sistema en SLEEP (Pulsa 3 o 4 para despertar)");
            drv_consumo_dormir();
        }
        else if (es_boton) {
            avanzar_secuencia_fin();
        }
    }
    // --- FASE SLEEP ---
    else {
        if (es_boton && (aux == BOTON_3 || aux == BOTON_4)) {
            LOG_MSG("Despertando por solicitud usuario!");
            reiniciar_juego();
            iniciar_secuencia_inicio();
        } else {
            // Falso despertar
            drv_consumo_dormir();
        }
    }
}

// ============================================================================
// LÓGICA DE JUEGO
// ============================================================================

static void generar_nuevo_compas(void) {
    uint8_t nuevo_patron = (nivel == 1) ? 
        ((hal_random(2) == 1) ? PATRON_LED_1 : PATRON_LED_2) : hal_random(4);
    compas[2] = nuevo_patron;
}

static void avanzar_compas(void) {
    compases_restantes--;
    compas_actual++;
    compas[0] = compas[1];
    compas[1] = compas[2];
    generar_nuevo_compas();
}

static bool verificar_fin_juego(void) {
    if (compases_restantes == 0) {
        LOG_MSG("Fin de juego: Completado!");
        return true;
    }
    if (puntuacion <= PUNTUACION_FALLO) {
        LOG_MSG("Fin de juego: Game Over por puntuacion");
        return true;
    }
    return false;
}

static void aumentar_dificultad_si_corresponde(void) {
    if ((NUM_COMPASES - compases_restantes) % 4 == 0 && nivel < 4) {
        nivel++;
        LOG_VAR("Nivel aumentado", nivel);
    }
}

static void evaluar_pulsacion(uint8_t boton_pulsado, uint32_t tiempo_reaccion) {
    if (entrada_valida) return;
    entrada_valida = true;

    if (patron_esperado_actual == PATRON_NINGUNO) {
        if (boton_pulsado == BOTON_1 || boton_pulsado == BOTON_2) {
            puntuacion -= 1;
            LOG_MSG("FALLO: Pulsacion innecesaria");
            #if DEBUG
            stats.pulsaciones_incorrectas++;
            #endif
            return;
        }
    }

    bool acierto = es_pulsacion_correcta(boton_pulsado);

    if (acierto) {
        int puntos = calcular_puntos_por_timing(tiempo_reaccion);
        procesar_acierto(puntos, tiempo_reaccion);
    } else {
        procesar_fallo();
    }
    LOG_VAR("Puntuacion actual", puntuacion);
}

static bool es_pulsacion_correcta(uint8_t boton_pulsado) {
    uint8_t boton_mask = (uint8_t)(1u << boton_pulsado);
    return (patron_esperado_actual & boton_mask) != 0;
}

static int calcular_puntos_por_timing(uint32_t tiempo_reaccion) {
    if (tiempo_reaccion <= VENTANA_PERFECTO_MS) return 2;
    else if (tiempo_reaccion <= VENTANA_ACIERTO_MS) return 1;
    else return 0;
}

static void procesar_acierto(int puntos, uint32_t tiempo_reaccion) {
    #if DEBUG
    sprintf(buffer, "[JUEGO] ACIERTO! Puntos: +%d (T: %d ms)\r\n", puntos, tiempo_reaccion);
    drv_uart_send(buffer);
    
    stats.compas_actual_acertado = true;
    stats.compas_actual_perfecto = (puntos == 2);
    stats.pulsaciones_correctas++;
    #endif
    puntuacion += puntos;
}

static void procesar_fallo(void) {
    LOG_MSG("FALLO! Boton incorrecto o a destiempo.");
    #if DEBUG
    stats.compas_actual_acertado = false;
    stats.compas_actual_perfecto = false;
    stats.pulsaciones_incorrectas++;
    #endif
    puntuacion -= 1;
}

static void evaluar_compas_sin_entrada(void) {
    if (!entrada_valida) {
        if (patron_esperado_actual == PATRON_NINGUNO) {
            puntuacion += 1;
            entrada_valida = true; 
            #if DEBUG
            stats.compas_actual_acertado = true;
            #endif
        } else {
            LOG_MSG("FALLO: Tiempo agotado sin respuesta");
            #if DEBUG
            stats.compases_sin_respuesta++;
            #endif
            puntuacion--;
            if (puntuacion < -10) puntuacion = -10;
        }
    }
}

// ============================================================================
// HARDWARE VISUAL
// ============================================================================

static void mostrar_transicion(void) { apagar_todos_leds(); }

static void mostrar_patron_final(void) {
    configurar_leds_patron(compas[2], compas[1]);
}

static void apagar_todos_leds(void) {
    drv_led_establecer(LED_1, LED_OFF);
    drv_led_establecer(LED_2, LED_OFF);
    drv_led_establecer(LED_3, LED_OFF);
    drv_led_establecer(LED_4, LED_OFF);
}

static void configurar_leds_patron(uint8_t patron_top, uint8_t patron_bottom) {
    drv_led_establecer(LED_1, (patron_top & 0x01) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_2, (patron_top & 0x02) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_3, (patron_bottom & 0x01) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_4, (patron_bottom & 0x02) ? LED_ON : LED_OFF);
}

static void configurar_leds_por_mask(uint8_t mask) {
    drv_led_establecer(LED_1, (mask & 0x01) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_2, (mask & 0x02) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_3, (mask & 0x04) ? LED_ON : LED_OFF);
    drv_led_establecer(LED_4, (mask & 0x08) ? LED_ON : LED_OFF);
}

static void iniciar_secuencia_fin(void) {
    etapa_secuencia_fin = 0;
    esperando_reinicio = false;
    apagar_todos_leds();
    
    // CORRECCIÓN PRINCIPAL: Reducido de 2000 a 200ms para feedback inmediato
    svc_alarma_activar(svc_alarma_codificar(false, 200, 0),
                      ev_PULSAR_BOTON, ID_TIMEOUT_INACTIVO);
}

static void avanzar_secuencia_fin(void) {
    apagar_todos_leds();
    configurar_leds_etapa_fin(etapa_secuencia_fin);
    etapa_secuencia_fin++;

    if (etapa_secuencia_fin == 11) {
        svc_alarma_activar(svc_alarma_codificar(false, 800, 0),
                          ev_PULSAR_BOTON, ID_TIMEOUT_FIN);
    } else {
        uint32_t delay = (etapa_secuencia_fin <= 11) ? 300 : 0;
        if (delay > 0) {
            svc_alarma_activar(svc_alarma_codificar(false, delay, 0),
                              ev_PULSAR_BOTON, ID_TIMEOUT_INACTIVO);
        }
    }
}

static void configurar_leds_etapa_fin(uint8_t etapa) {
    uint8_t mask = (etapa < sizeof(FIN_MASK)) ? FIN_MASK[etapa] : 0x00;
    configurar_leds_por_mask(mask);
}

// ============================================================================
// ESTADÍSTICAS
// ============================================================================

#if DEBUG
static void inicializar_estadisticas(void) {
    stats.compases_acertados = 0;
    stats.compases_fallados = 0;
    stats.compases_perfectos = 0;
    stats.compases_sin_respuesta = 0;
    stats.pulsaciones_correctas = 0;
    stats.pulsaciones_incorrectas = 0;
    stats.total_compases_activos = 0;
    stats.aciertos_activos = 0;

    resetear_estadisticas_compas_actual();
}

static void resetear_estadisticas_compas_actual(void) {
    stats.compas_actual_acertado = false;
    stats.compas_actual_perfecto = false;
    entrada_valida = false;
}

static void actualizar_estadisticas_compas(void) {
    if (!entrada_valida) return;
    if (compas_actual < 1 || compas_actual > (NUM_COMPASES - 2)) return;

    if (stats.compas_actual_acertado) {
        stats.compases_acertados++;
        if (stats.compas_actual_perfecto) {
            stats.compases_perfectos++;
        }
    } else {
        stats.compases_fallados++;
    }

    if (patron_esperado_actual != PATRON_NINGUNO) {
        stats.total_compases_activos++;
        if (stats.compas_actual_acertado) {
            stats.aciertos_activos++;
        }
    }
}

static void mostrar_estadisticas_finales(void) {
    float porcentaje_precision = calcular_porcentaje(stats.aciertos_activos, stats.total_compases_activos);
    float porcentaje_perfectos = calcular_porcentaje(stats.compases_perfectos, stats.total_compases_activos);

    LOG_MSG("=== ESTADISTICAS (SKILL REAL) ===");

    sprintf(buffer, "[STATS] Activos: %d | Hits Activos: %d\r\n", stats.total_compases_activos, stats.aciertos_activos); drv_uart_send(buffer);
    sprintf(buffer, "[STATS] Precision: %.1f%%\r\n", porcentaje_precision); drv_uart_send(buffer);
    sprintf(buffer, "[STATS] Perfectos: %d\r\n", stats.compases_perfectos); drv_uart_send(buffer);
    sprintf(buffer, "[STATS] Puntuacion: %d\r\n", puntuacion); drv_uart_send(buffer);

    const char* rendimiento = evaluar_rendimiento(porcentaje_precision);
    LOG_MSG(rendimiento);
}

static float calcular_porcentaje(uint8_t valor, uint8_t total) {
    if (total == 0) return 0.0f;
    return (valor / (float)total) * 100.0f;
}

static const char* evaluar_rendimiento(float porcentaje_aciertos) {
    if (porcentaje_aciertos >= 90 && stats.compases_perfectos >= 3) return "Rango: S (EXCELENTE)";
    else if (porcentaje_aciertos >= 75) return "Rango: A (MUY BUENO)";
    else if (porcentaje_aciertos >= 60) return "Rango: B (BUENO)";
    else if (porcentaje_aciertos >= 40) return "Rango: C (REGULAR)";
    else return "Rango: D (MEJORABLE)";
}
#endif
