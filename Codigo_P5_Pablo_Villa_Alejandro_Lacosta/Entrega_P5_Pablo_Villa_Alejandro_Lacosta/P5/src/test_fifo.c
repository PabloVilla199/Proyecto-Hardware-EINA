/* *****************************************************************************
 * P.H.2025: test_fifo.c
 *
 * Pruebas funcionales del módulo rt_fifo
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Comprueba la correcta operación de la cola FIFO de eventos:
 * - Inicialización
 * - Encolado y extracción
 * - Estado vacío y lleno
 * - Estadísticas de eventos
 * 
 * Los resultados se muestran mediante LEDs de estado.
 *
 ******************************************************************************/


#include "test_fifo.h"
#include <stdbool.h>
#include <stdint.h>
#include "rt_fifo.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"


/* ----------------- CONFIGURACIÓN LEDS ----------------- */
#define LED_FIFO_OK        1
#define LED_FIFO_FAIL      2
#define LED_FIFO_FULL      3
#define LED_FIFO_STATS     4

#define TIEMPO_ESPERA_MS   1500  // Pausa entre tests

/* ----------------- HELPERS ----------------- */
static bool TEST_ASSERT(bool condition) {
    return condition;
}

/* ----------------- TEST 1: Inicialización ----------------- */
static bool test_inicializar(void) {
    rt_FIFO_inicializar(4);
    return TEST_ASSERT(rt_FIFO_estadisticas(ev_VOID) == 0);
}

/* ----------------- TEST 2: Encolado y extracción ----------------- */
static bool test_encolar_extraer(void) {
    rt_FIFO_inicializar(4);

    EVENTO_T ids[3] = { ev_T_PERIODICO, ev_PULSAR_BOTON, ev_T_PERIODICO };
    uint32_t aux[3] = { 10, 20, 30 };
    for (int i = 0; i < 3; i++) {
        rt_FIFO_encolar(ids[i], aux[i]);
    }

    for (int i = 0; i < 3; i++) {
        EVENTO_T id;
        uint32_t a;
        Tiempo_us_t ts;
        rt_FIFO_extraer(&id, &a, &ts);
        if (id != ids[i] || a != aux[i]) return false;
    }
    return TEST_ASSERT(rt_FIFO_estadisticas(ev_VOID) == 0);
}

/* ----------------- TEST 3: Cola llena ----------------- */
static bool test_fifo_llena(void) {
    rt_FIFO_inicializar(4);
    for (int i = 0; i < RT_FIFO_TAM; i++) {
        rt_FIFO_encolar(ev_T_PERIODICO, i);
    }
    return TEST_ASSERT(rt_FIFO_estadisticas(ev_VOID) == RT_FIFO_TAM);
}

/* ----------------- TEST 4: Estadísticas ----------------- */
static bool test_estadisticas(void) {
    rt_FIFO_inicializar(4);
    rt_FIFO_encolar(ev_T_PERIODICO, 1);
    rt_FIFO_encolar(ev_T_PERIODICO, 2);
    rt_FIFO_encolar(ev_PULSAR_BOTON, 3);
    return TEST_ASSERT(rt_FIFO_estadisticas(ev_T_PERIODICO) == 2 &&
                       rt_FIFO_estadisticas(ev_PULSAR_BOTON) == 1);
}


/* ----------------- WRAPPER GENERAL ----------------- */
bool test_fifo_run(void) {
    drv_leds_iniciar();
    drv_tiempo_iniciar();

    bool ok1 = test_inicializar();
    bool ok2 = test_encolar_extraer();
    bool ok3 = test_fifo_llena();
    bool ok4 = test_estadisticas();

    drv_led_establecer(LED_FIFO_OK,     ok1 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_FIFO_FAIL,   ok2 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_FIFO_FULL,   ok3 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_FIFO_STATS,  ok4 ? LED_ON : LED_OFF);


    while (1) {
        drv_consumo_dormir();  // Mantener estado para inspección
    }

}
