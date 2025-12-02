/* *****************************************************************************
 * P.H.2025: blink.c
 *
 * M?dulo de implementaci?n de las distintas estrategias de parpadeo y tests.
 *
 * - Blink v1: Espera activa "quemando" ciclos de CPU en bucle. Bloqueante e ineficiente.
 * - Blink v2: Espera activa mediante consulta al driver de tiempo. Preciso pero bloqueante.
 * - Blink v3: Basado en interrupciones (timer peri?dico + callback). Permite ahorro de energ?a (Idle).
 * - Blink v3_bis: Variante de v3 que entra en modo "Dormir Profundo" (Deep Sleep) tras 20 ciclos.
 * - Blink v4: Arquitectura completa orientada a eventos (Cola FIFO + Dispatcher). No bloqueante.
 *
 * Autores: Alejandro Lacosta y Pablo Villa
 * Universidad de Zaragoza
 * ****************************************************************************/

#include "blink.h"

#include <stdint.h>
#include <stdbool.h>
#include "hal_gpio.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "drv_uart.h"
#include "svc_logs.h"
#include "rt_fifo.h"


#define LED_1   ((LED_id_t)1)
#define LED_2   ((LED_id_t)2)
#define LED_3   ((LED_id_t)3)
#define LED_4   ((LED_id_t)4)

#define RETARDO_MS 500u
#ifndef MONITOR4
#define MONITOR4 4
#endif

static LED_id_t blink3_led_id;
static volatile int contador_blink3_bis = 0;


/* --------------------------------------------------------------------------
 * BLINK V1
 * -------------------------------------------------------------------------- */
static void blink_v1(LED_id_t id){
    drv_led_establecer(id, LED_ON);
    while(1) {
        volatile uint32_t tmo = 10000000u;
        while(tmo--);
        drv_led_conmutar(id);
    }
}


/* --------------------------------------------------------------------------
 * BLINK V2
 * -------------------------------------------------------------------------- */
static void blink_v2(LED_id_t id) {
    Tiempo_ms_t t = drv_tiempo_actual_ms();
    drv_led_establecer(id, LED_ON);
    while(1) {
        t += RETARDO_MS;
        drv_tiempo_esperar_hasta_ms(t);
        drv_led_conmutar(id);
    }
}


/* --------------------------------------------------------------------------
 * BLINK V3
 * -------------------------------------------------------------------------- */
static void blink3_callback(void) { drv_led_conmutar(blink3_led_id); }

static void blink_v3(LED_id_t id) {
    drv_led_establecer(id, LED_OFF);
    blink3_led_id = id;
    drv_tiempo_periodico_ms(RETARDO_MS, blink3_callback, id);
    while(1) drv_consumo_esperar();
}


/* --------------------------------------------------------------------------
 * BLINK V3 BIS
 * -------------------------------------------------------------------------- */
static void blink3_bis_callback(void) {
    drv_led_conmutar(blink3_led_id);
    contador_blink3_bis++;
    if (contador_blink3_bis >= 20) {
        drv_led_establecer(blink3_led_id, LED_OFF);
        drv_consumo_dormir();
    }
}

static void blink_v3_bis(LED_id_t id) {
    contador_blink3_bis = 0;
    drv_led_establecer(id, LED_OFF);
    blink3_led_id = id;
    drv_tiempo_periodico_ms(RETARDO_MS, blink3_bis_callback, id);
    while(1) drv_consumo_esperar();
}


/* --------------------------------------------------------------------------
 * BLINK V4
 * -------------------------------------------------------------------------- */
static void blink_v4(LED_id_t id) {
    EVENTO_T id_evento;
    uint32_t auxData;
    Tiempo_us_t ts;

    rt_FIFO_inicializar(MONITOR4);
    drv_led_establecer(id, LED_OFF);

    for (int i = 0; i < 60; i++) {
        rt_FIFO_encolar(ev_T_PERIODICO, id);
    }

    while(1) {
        if(rt_FIFO_extraer(&id_evento, &auxData, &ts)){
            switch(id_evento) {
                case ev_T_PERIODICO:
                    drv_led_conmutar((LED_id_t)auxData);
                    drv_tiempo_esperar_ms(RETARDO_MS);
                    break;

                case ev_PULSAR_BOTON:
                    drv_led_conmutar(LED_1); // Ejemplo uso define
                    drv_consumo_esperar();
                    break;

                case ev_VOID:
                default:
                    drv_consumo_esperar();
                    break;
            }
        } else {
            drv_consumo_esperar();
        }
    }
}


/* --------------------------------------------------------------------------
 * DISPATCHER P?BLICO
 * -------------------------------------------------------------------------- */
void blink_ejecutar(uint8_t sesion) {

   switch(sesion) {
       case 1: blink_v1(LED_1); break;
       case 2: blink_v2(LED_2); break;
       case 3: blink_v3(LED_3); break;
       case 4: blink_v4(LED_4); break;
       case 5: blink_v3_bis(LED_3); break;
       default: while(1) drv_consumo_esperar();
  }
}
