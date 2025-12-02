/******************************************************************************
* Fichero: beat_hero.c
 * Proyecto: Guitar Hero - Prácticas P.H. 2025
 *
 * Descripción:
 *   Implementación del juego "Beat Hero" basado en una secuencia aleatoria de compases
 *    que el jugador debe reproducir correctamente. Incluye la máquina de
 *   estados, gestión de tiempos, uso de alarmas y lectura de botones.
 * 
 *  Version Basica del juego
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/
#include "beat_hero.h"
#include "drv_leds.h"
#include "drv_botones.h"
#include "drv_tiempo.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "hal_random.h"
#include "rt_GE.h"
#include <stddef.h>

// ============================================================================
// CONSTANTES Y DEFINICIONES
// ============================================================================

#define NUM_COMPASES        15
#define BPM_INICIAL         90
#define COMPAS_MS           (60000 / BPM_INICIAL)
#define VENTANA_ACIERTO_MS  (COMPAS_MS * 0.4)
#define TIEMPO_ENTRE_COMPASES 100
#define PUNTUACION_FALLO    -5
#define TIEMPO_INACTIVIDAD  10000

// IDs de timeouts
#define ID_TIMEOUT_COMPAS   0x01
#define ID_TIMEOUT_INACTIVO 0x02
#define ID_TIMEOUT_FIN      0x03
#define ID_TIMEOUT_INICIO   0x04

// LEDs
#define LED_1   1
#define LED_2   2  
#define LED_3   3
#define LED_4   4

// Patrones
#define PATRON_LED_1        0x01
#define PATRON_LED_2        0x02
#define PATRON_AMBOS        0x03
#define PATRON_NINGUNO      0x00

#define BOTON_1             0
#define BOTON_2             1
#define BOTON_3             2
#define BOTON_4             3

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

static uint8_t compas[3];
static uint8_t compases_restantes;
static estado_juego_t estado_actual;
static int32_t puntuacion;
static uint32_t tiempo_inicio_compas;
static uint8_t nivel;
static uint8_t patron_esperado_actual;
static bool esperando_reinicio;

// ============================================================================
// FUNCIONES PRIVADAS
// ============================================================================

static void juego_fsm(EVENTO_T ev, uint32_t aux);
static void generar_nuevo_compas(void);
static void avanzar_compas(void);
static bool verificar_fin_juego(void);
static void evaluar_pulsacion(uint8_t boton_pulsado);
static bool es_pulsacion_correcta(uint8_t boton_pulsado);
static void mostrar_patron_actual(void);
static void apagar_todos_leds(void);
static void configurar_leds_patron(uint8_t patron_top, uint8_t patron_bottom);
static void iniciar_secuencia_fin(void);
static void reiniciar_juego(void);
static void dormir_sistema(void);
static void iniciar_secuencia_inicio(void);

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

void beat_hero_iniciar(void) {
    // Inicializar Gestor de Eventos
    rt_GE_iniciar(5);
    
    // Inicializar drivers
    drv_leds_iniciar();
    drv_botones_iniciar(NULL, ev_PULSAR_BOTON, ev_PULSAR_BOTON);
    hal_random_iniciar(drv_tiempo_actual_ms());

    // Suscribir a eventos
    svc_GE_suscribir(ev_PULSAR_BOTON, 2, juego_fsm);
    svc_GE_suscribir(ev_BEAT_TIMEOUT, 2, juego_fsm);

    reiniciar_juego();
    rt_GE_lanzador();
}

static void reiniciar_juego(void) {
    estado_actual = e_INIT;
    puntuacion = 0;
    nivel = 1;
    compases_restantes = NUM_COMPASES;
    esperando_reinicio = false;

    // Inicializar compases
    for(int i = 0; i < 3; i++) compas[i] = 0;
    for(int i = 0; i < 3; i++) generar_nuevo_compas();
    
    apagar_todos_leds();
    iniciar_secuencia_inicio();
}

static void iniciar_secuencia_inicio(void) {
    // Secuencia de inicio simple: parpadeo de todos los LEDs
    drv_led_establecer(LED_1, LED_ON);
    drv_led_establecer(LED_2, LED_ON);
    drv_led_establecer(LED_3, LED_ON);
    drv_led_establecer(LED_4, LED_ON);
    
    svc_alarma_activar(svc_alarma_codificar(false, 500, 0), 
                      ev_BEAT_TIMEOUT, ID_TIMEOUT_INICIO);
}

// ============================================================================
// MÁQUINA DE ESTADOS PRINCIPAL
// ============================================================================

static void juego_fsm(EVENTO_T ev, uint32_t aux) {
    const bool es_timeout = (ev == ev_BEAT_TIMEOUT);
    const bool es_boton = (ev == ev_PULSAR_BOTON && aux < 4);
    const bool es_boton_salida = (es_boton && (aux == BOTON_3 || aux == BOTON_4));

    // Reiniciar timeout de inactividad en cualquier evento
    svc_alarma_activar(svc_alarma_codificar(false, TIEMPO_INACTIVIDAD, 0),
                      ev_BEAT_TIMEOUT, ID_TIMEOUT_INACTIVO);

    switch (estado_actual) {
        case e_INIT:
            if (es_timeout && aux == ID_TIMEOUT_INICIO) {
                apagar_todos_leds();
                estado_actual = e_SHOW_SEQUENCE;
                svc_alarma_activar(svc_alarma_codificar(false, TIEMPO_ENTRE_COMPASES, 0),
                                  ev_BEAT_TIMEOUT, ID_TIMEOUT_COMPAS);
            }
            break;

        case e_SHOW_SEQUENCE:
            if (es_timeout && aux == ID_TIMEOUT_COMPAS) {
                mostrar_patron_actual();
                patron_esperado_actual = compas[0];
                tiempo_inicio_compas = drv_tiempo_actual_ms();
                estado_actual = e_WAIT_FOR_INPUT;
                
                svc_alarma_activar(svc_alarma_codificar(false, COMPAS_MS, 0),
                                  ev_BEAT_TIMEOUT, ID_TIMEOUT_COMPAS);
            }
            else if (es_boton_salida) {
                puntuacion = PUNTUACION_FALLO - 1;
                estado_actual = e_FIN_PARTIDA;
                iniciar_secuencia_fin();
            }
            break;

        case e_WAIT_FOR_INPUT:
            if (es_boton) {
                evaluar_pulsacion((uint8_t)aux);
            }
            else if (es_timeout && aux == ID_TIMEOUT_COMPAS) {
                // Timeout del compás - evaluar si no hubo entrada
                if (patron_esperado_actual != PATRON_NINGUNO) {
                    puntuacion--;
                }

                avanzar_compas();

                if (verificar_fin_juego()) {
                    estado_actual = e_FIN_PARTIDA;
                    iniciar_secuencia_fin();
                } else {
                    estado_actual = e_SHOW_SEQUENCE;
                    svc_alarma_activar(svc_alarma_codificar(false, TIEMPO_ENTRE_COMPASES, 0),
                                      ev_BEAT_TIMEOUT, ID_TIMEOUT_COMPAS);
                }
            }
            else if (es_boton_salida) {
                puntuacion = PUNTUACION_FALLO - 1;
                estado_actual = e_FIN_PARTIDA;
                iniciar_secuencia_fin();
            }
            break;

        case e_FIN_PARTIDA:
            if (es_timeout && aux == ID_TIMEOUT_FIN) {
                apagar_todos_leds();
                esperando_reinicio = true;
                dormir_sistema();
            }
            else if (es_boton_salida && esperando_reinicio) {
                reiniciar_juego();
            }
            break;
    }

    // Verificar timeout de inactividad global
    if (es_timeout && aux == ID_TIMEOUT_INACTIVO) {
        dormir_sistema();
    }
}

// ============================================================================
// LÓGICA DEL JUEGO
// ============================================================================

static void generar_nuevo_compas(void) {
    uint8_t nuevo_patron;
    
    switch(nivel) {
        case 1:
            // Solo un LED encendido por compás
            nuevo_patron = (hal_random(2) == 1) ? PATRON_LED_1 : PATRON_LED_2;
            break;
        case 2:
            // Se permiten compases sin LEDs
            nuevo_patron = hal_random(4); // 0-3
            if (nuevo_patron == 3) nuevo_patron = PATRON_AMBOS;
            break;
        case 3:
        case 4:
            // Permitir todos los patrones
            nuevo_patron = hal_random(4);
            break;
        default:
            nuevo_patron = PATRON_NINGUNO;
    }
    
    compas[2] = nuevo_patron;
}

static void avanzar_compas(void) {
    compases_restantes--;
    compas[0] = compas[1];
    compas[1] = compas[2];
    generar_nuevo_compas();

    // Aumentar dificultad cada 8 compases
    if ((NUM_COMPASES - compases_restantes) % 8 == 0 && nivel < 4) {
        nivel++;
    }
}

static bool verificar_fin_juego(void) {
    return (compases_restantes == 0 || puntuacion <= PUNTUACION_FALLO);
}

static void evaluar_pulsacion(uint8_t boton_pulsado) {
    if (patron_esperado_actual == PATRON_NINGUNO) {
        if (boton_pulsado == BOTON_1 || boton_pulsado == BOTON_2) {
            puntuacion -= 1;
        }
        return;
    }

    bool acierto = es_pulsacion_correcta(boton_pulsado);
    uint32_t tiempo_reaccion = drv_tiempo_actual_ms() - tiempo_inicio_compas;

    if (acierto) {
        if (tiempo_reaccion <= VENTANA_ACIERTO_MS) {
            puntuacion += (tiempo_reaccion <= (VENTANA_ACIERTO_MS * 0.5)) ? 2 : 1;
        }
    } else {
        puntuacion -= 1;
    }
}

static bool es_pulsacion_correcta(uint8_t boton_pulsado) {
    uint8_t boton_mask = (uint8_t)(1u << boton_pulsado);
    return (patron_esperado_actual & boton_mask) != 0;
}

// ============================================================================
// VISUALIZACIÓN
// ============================================================================

static void mostrar_patron_actual(void) {
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

static void iniciar_secuencia_fin(void) {
    // Secuencia de fin simple: LEDs se apagan en secuencia
    drv_led_establecer(LED_1, LED_ON);
    drv_led_establecer(LED_2, LED_ON);
    drv_led_establecer(LED_3, LED_ON);
    drv_led_establecer(LED_4, LED_ON);
    
    svc_alarma_activar(svc_alarma_codificar(false, 100, 0),
                      ev_BEAT_TIMEOUT, ID_TIMEOUT_FIN);
}

// ============================================================================
// GESTIÓN DE ENERGÍA
// ============================================================================

static void dormir_sistema(void) {
    apagar_todos_leds();
		drv_consumo_dormir();
    
    esperando_reinicio = true;
}
