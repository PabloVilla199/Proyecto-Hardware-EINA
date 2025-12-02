/******************************************************************************
 * Fichero: drv_botones_test.c
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Módulo de pruebas para el sistema de gestión de botones integrado con
 *   el gestor de eventos. Verifica que las interrupciones externas generan
 *   eventos correctos y que cada pulsación enciende el LED correspondiente.
 *
 * Autores:
 * Alejandro Lacosta 
 * Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "drv_botones_test.h"
#include <stdint.h>
#include "drv_botones.h"
#include "drv_leds.h"
#include "svc_GE.h"
#include "rt_GE.h"
#include "drv_uart.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "svc_logs.h"
#include <stdbool.h>
#include <stdio.h>

#define NUM_TEST_BOTONES NUM_BOTONES

volatile bool sistema_listo = false;

/**
 * @brief Callback ejecutado cuando se detecta un evento de pulsación de botón.
 *
 * Muestra información por UART, registra logs y enciende el LED asociado al
 * botón cuyo ID viene en auxData.
 */
static void test_boton_cb(EVENTO_T ev, uint32_t aux){
    hal_ext_int_id_t id = (hal_ext_int_id_t) aux;
    LOG_INFO("Callback de botón activado para id:");
    char buf[4]; sprintf(buf, "%u", id); drv_uart_send(buf); drv_uart_send("\r\n");

    if (ev == ev_PULSAR_BOTON) {
        drv_led_establecer((LED_id_t)id + 1, LED_ON);
        LOG_INFO("LED encendido por botón:");
        drv_uart_send(buf); drv_uart_send("\r\n");
    }
}

/**
 * @brief Ejecuta el test completo del gestor de eventos con botones y LEDs.
 *
 * Inicializa el sistema, suscribe callbacks, habilita el gestor de eventos
 * y entra en el bucle de procesamiento continuo.
 */
void test_GE_botones_leds(void){
    drv_tiempo_iniciar();
    drv_uart_init();
    drv_leds_iniciar();
    drv_consumo_iniciar();
    svc_logs_iniciar();
    LOG_INFO("=== INICIANDO TEST DE GE + BOTONES + LEDS ===");

    rt_GE_iniciar(10);
    LOG_INFO("Gestor de eventos iniciado");

    drv_botones_iniciar(test_boton_cb, ev_PULSAR_BOTON, ev_BOTON_RETARDO);
    LOG_INFO("Driver de botones iniciado");
    
    svc_GE_suscribir(ev_PULSAR_BOTON, 2, test_boton_cb);

    sistema_listo = true;
    LOG_INFO("Sistema listo. Pulsa un botón para encender LED correspondiente.");

    rt_GE_lanzador();
}

/**
 * @brief Punto de entrada principal del test de botones + GE.
 * @return Siempre 0 (la ejecución real nunca vuelve del lanzador GE)
 */
int test_GE(void){
    test_GE_botones_leds();
    return 0;
}
