/* *****************************************************************************
 * P.H.2025: main.c
 *
 * Programa principal (Punto de entrada).
 * Se encarga de la inicializaci�n m�nima del hardware y delega el control
 * completo al juego Beat Hero.
 *
 * Autores: Alejandro Lacosta y Pablo Villa
 * Universidad de Zaragoza
 * ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "hal_gpio.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "drv_wdt.h"
#include "test.h"

#define RUN_MODE 0
#define DEBUG 1

#if DEBUG
#include "drv_uart.h"
#include "svc_logs.h"
#endif

#include "beat_hero.h"
#include "blink.h"
#include "bit_counter_strike.h"

#ifndef TEST_ID
#define TEST_ID 2
#endif

#ifndef BLINK_SESSION
#define BLINK_SESSION 3
#endif


int main(void) {

    drv_wdt_iniciar(5);
    drv_tiempo_iniciar();
    hal_gpio_iniciar();
    drv_leds_iniciar();

#if DEBUG
    drv_uart_init();
    svc_logs_iniciar();
    LOG_INFO("=== SISTEMA INICIADO ===");
#endif

    switch (RUN_MODE) {

        case 0:
#if DEBUG
            LOG_INFO("=== MODO BEAT_HERO ===");
#endif
            beat_hero_extend_iniciar();
            while (1) drv_consumo_esperar();

        case 1:
#if DEBUG
            LOG_INFO("=== MODO TESTS ===");
#endif
            ejecutar_sesion_test(TEST_ID);
            while (1) drv_consumo_esperar();

        case 2:
            blink_ejecutar(BLINK_SESSION);
            while (1) drv_consumo_esperar();

        case 3:
#if DEBUG
            LOG_INFO("=== MODO BITCOUNTER ===");
#endif
            bit_counter_strike_iniciar();
            while (1) drv_consumo_esperar();
            
        default:
            while (1) drv_consumo_esperar();
    }
}
