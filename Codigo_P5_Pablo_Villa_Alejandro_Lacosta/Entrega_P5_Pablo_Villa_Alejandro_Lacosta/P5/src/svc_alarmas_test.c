/******************************************************************************
 * Fichero: svc_alarmas_test.c
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Módulo de pruebas del servicio de alarmas. Configura temporizadores,
 *   activa alarmas de tipo único y periódicas y muestra por UART el resultado
 *   de los disparos, incluyendo conmutación de LEDs.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "svc_alarmas_test.h"
#include <stdint.h>
#include <stdbool.h>
#include "svc_alarmas.h"
#include "drv_tiempo.h"
#include "drv_leds.h"
#include "drv_uart.h"
#include <stdio.h>

#define LED1 1
#define LED2 2
#define LED3 3

static volatile int contador_periodico = 0;

/**
 * @brief Callback de prueba. Conmuta LEDs y envía mensajes por UART.
 */
static void test_callback(EVENTO_T ev, uint32_t auxData) {
    drv_led_conmutar((LED_id_t)auxData);

    if (auxData == LED2) contador_periodico++;
}

/**
 * @brief Inicializa el entorno de pruebas y activa varias alarmas de ejemplo.
 */
void svc_alarmas_test_iniciar(void) {
    drv_tiempo_iniciar();
    drv_leds_iniciar();
    drv_uart_init();

    // Inicializar el servicio de alarmas con tick cada 250ms
    svc_alarma_iniciar(0, test_callback, ev_T_PERIODICO);

    drv_uart_send("[TEST] Activando alarma única LED1 (1s)\r\n");
    uint32_t cod_unica = svc_alarma_codificar(false, 1000, 0);
    svc_alarma_activar(cod_unica, ev_T_PERIODICO, LED1);

    drv_uart_send("[TEST] Activando alarma periódica LED2 (500ms)\r\n");
    uint32_t cod_periodica = svc_alarma_codificar(true, 500, 0);
    svc_alarma_activar(cod_periodica, ev_T_PERIODICO, LED2);

    drv_uart_send("[TEST] Activando otra alarma única LED3 (1500ms)\r\n");
    uint32_t cod_unica2 = svc_alarma_codificar(false, 1500, 0);
    svc_alarma_activar(cod_unica2, ev_T_PERIODICO, LED3);

    for (int i = 0; i < 50; i++) {
        //drv_consumo_esperar();
    }

    drv_uart_send("[TEST] Desactivando alarma periódica LED2\r\n");
}
