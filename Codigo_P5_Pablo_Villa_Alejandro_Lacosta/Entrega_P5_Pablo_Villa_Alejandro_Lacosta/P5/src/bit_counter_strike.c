/******************************************************************************
 * Fichero: bit_counter_strike.c
 * Proyecto: Guitar Hero - Pr?cticas P.H. 2025
 *
 * Descripci?n:
 *   Implementaci?n del juego "Counter Strike" basado en una secuencia fija de
 *   LEDs que el jugador debe reproducir correctamente. Incluye la m?quina de
 *   estados, gesti?n de tiempos, uso de alarmas y lectura de botones.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "bit_counter_strike.h"
#include "drv_leds.h"
#include "drv_botones.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include <stddef.h>

#define TAMANO_SECUENCIA    8
#define TIEMPO_BASE_MS      1500u
#define TIEMPO_PAUSA_MS     500u
#define TIEMPO_BLINK_MS     200u
#define ID_TIMEOUT          0xFF

static const uint8_t SECUENCIA[TAMANO_SECUENCIA] = {1, 3, 2, 4, 1, 4, 2, 3};

/* Estados de la m?quina de juego */
typedef enum {
    e_INIT,
    e_SHOW_SEQUENCE,
    e_WAIT_FOR_INPUT,
    e_FIN_PARTIDA
} estado_juego_t;

static estado_juego_t estado_actual;
static uint8_t  indice_secuencia;
static uint32_t tiempo_limite;
static uint8_t  contador_parpadeos;

static void juego_fsm(EVENTO_T ev, uint32_t aux);
static void programar_alarma(uint32_t ms);
static void cancelar_alarma(void);

/**
 * Inicializa todo el sistema de juego.
 * Prepara los LEDs, los botones, resetea estados y programa la primera alarma.
 */
void bit_counter_strike_iniciar(void) {
    drv_leds_iniciar();
    drv_botones_iniciar(NULL, ev_PULSAR_BOTON, ev_PULSAR_BOTON);

    estado_actual = e_INIT;
    contador_parpadeos = 0;
    indice_secuencia = 0;
    tiempo_limite = TIEMPO_BASE_MS;

    svc_GE_suscribir(ev_PULSAR_BOTON, 2, juego_fsm);

    programar_alarma(10);
}

/**
 * M?quina de estados del juego Counter Strike.
 * Gestiona el flujo completo: animaci?n inicial, mostrar secuencia, entrada
 * del jugador, evaluaci?n, victoria/derrota y reinicio.
 */
static void juego_fsm(EVENTO_T ev, uint32_t aux) {
    bool es_timeout = (aux == ID_TIMEOUT);
    bool es_boton = (aux < 4);

    switch (estado_actual) {

    case e_INIT:
        if (es_timeout) {
            if (contador_parpadeos < 6) {
                for (int i = 1; i <= 4; i++) {
                    drv_led_conmutar((LED_id_t)i);
                }
                contador_parpadeos++;
                programar_alarma(TIEMPO_BLINK_MS);
            } else {
                for (int i = 1; i <= 4; i++) {
                    drv_led_establecer((LED_id_t)i, LED_OFF);
                }
                estado_actual = e_SHOW_SEQUENCE;
                programar_alarma(1000);
            }
        }
        break;

    case e_SHOW_SEQUENCE:
        if (es_timeout) {
            uint8_t led_objetivo = SECUENCIA[indice_secuencia];

            drv_led_establecer((LED_id_t)led_objetivo, LED_ON);

            programar_alarma(tiempo_limite);
            estado_actual = e_WAIT_FOR_INPUT;
        }
        break;

    case e_WAIT_FOR_INPUT:

        if (es_boton) {
            uint8_t boton_pulsado = (uint8_t)aux;
            uint8_t led_jugado = boton_pulsado + 1;
            uint8_t led_objetivo = SECUENCIA[indice_secuencia];

            if (led_jugado == led_objetivo) {
                drv_led_establecer((LED_id_t)led_objetivo, LED_OFF);
                cancelar_alarma();

                indice_secuencia++;

                if (tiempo_limite > 400) {
                    tiempo_limite -= 100;
                }

                if (indice_secuencia >= TAMANO_SECUENCIA) {
                    estado_actual = e_FIN_PARTIDA;
                    programar_alarma(100);
                } else {
                    estado_actual = e_SHOW_SEQUENCE;
                    programar_alarma(TIEMPO_PAUSA_MS);
                }

            } else {
                cancelar_alarma();
                estado_actual = e_FIN_PARTIDA;
                programar_alarma(100);
            }
        } else if (es_timeout) {
            uint8_t led_objetivo = SECUENCIA[indice_secuencia];
            drv_led_establecer((LED_id_t)led_objetivo, LED_OFF);

            estado_actual = e_FIN_PARTIDA;
            programar_alarma(100);
        }
        break;

    case e_FIN_PARTIDA:
        if (es_timeout) {
            for (int i = 1; i <= 4; i++) {
                drv_led_establecer((LED_id_t)i, LED_OFF);
            }

            indice_secuencia = 0;
            contador_parpadeos = 0;
            tiempo_limite = TIEMPO_BASE_MS;

            estado_actual = e_INIT;
            programar_alarma(2000);
        }
        break;
    }
}

/**
 * Programa una alarma con el tiempo indicado en milisegundos.
 * El timeout generar? un evento que ser? gestionado por la FSM del juego.
 */
static void programar_alarma(uint32_t ms) {
    uint32_t flags = svc_alarma_codificar(false, ms, 0);
    svc_alarma_activar(flags, ev_PULSAR_BOTON, ID_TIMEOUT);
}

/**
 * Cancela la alarma activa asociada al juego.
 */
static void cancelar_alarma(void) {
    svc_alarma_activar(0, ev_PULSAR_BOTON, ID_TIMEOUT);
}

