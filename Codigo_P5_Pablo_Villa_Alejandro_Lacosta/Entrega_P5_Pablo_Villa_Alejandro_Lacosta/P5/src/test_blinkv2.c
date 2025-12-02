/* *****************************************************************************
 * P.H.2025: test_blink_v2.c
 *
 * Pruebas funcionales del módulo blink2
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Comprueba la correcta operación del blink_v2:
 * 
 * Los resultados se muestran mediante LEDs de estado.
 *
 ******************************************************************************/

#include "test_blink_v2.h"

/* ----------------- CONFIGURACIÓN ----------------- */
#define LED_ENCENDER      1
#define LED_APAGAR        2
#define LED_CONMUTAR      3
#define LED_TEST_OK       4
#define TIEMPO_ESPERA_MS  2000   // 2 segundos = 2000 ms para pruebas

/* ----------------- PARÁMETRO GLOBAL ----------------- */
#define CICLOS_POR_MS     1600u   // Para 1 ms en CPU 16MHz

/* ----------------- HELPERS ----------------- */
static bool TEST_ASSERT(bool condition) {
    return condition;
}

/* ----------------- TESTS ----------------- */
static bool test_encender_led(uint8_t led_id) {
    drv_led_establecer(led_id, LED_ON);
    LED_status_t st;
    drv_led_estado(led_id, &st);
    return TEST_ASSERT(st == LED_ON);
}

static bool test_apagar_led(uint8_t led_id) {
    drv_led_establecer(led_id, LED_OFF);
    LED_status_t st;
    drv_led_estado(led_id, &st);
    return TEST_ASSERT(st == LED_OFF);
}

static bool test_conmutar_led(uint8_t led_id) {
    drv_led_establecer(led_id, LED_OFF);
    drv_led_conmutar(led_id);
    LED_status_t st;
    drv_led_estado(led_id, &st);
    return TEST_ASSERT(st == LED_ON);
}

/* ----------------- RETARDO SOFTWARE EN MILISEGUNDOS ----------------- */

static void delay_software_ms(uint32_t milisegundos) {
    for (uint32_t ms = 0; ms < milisegundos; ms++) {
        // Bucle para aproximadamente 1 milisegundo
        for (volatile uint32_t i = 0; i < CICLOS_POR_MS; i++) {
            // Retardo de 1 ms
        }
    }
}

static bool test_esperar_ms(uint32_t milisegundos) {
	
    uint32_t t_start_drv = drv_tiempo_actual_ms();
    drv_tiempo_esperar_ms(milisegundos);  // Driver en ms
    uint32_t t_end_drv = drv_tiempo_actual_ms();
    uint32_t duracion_drv = t_end_drv - t_start_drv;

    // Comparamos con margen fijo de ±20 ms
    bool ok = (duracion_drv >= (milisegundos - 20)) && 
              (duracion_drv <= (milisegundos + 20));
    return TEST_ASSERT(ok);
}
static void calibrar_ciclos_por_ms(void) {
    // Tomar tiempo inicial
    uint32_t t_start = drv_tiempo_actual_ms();
    
    // Ejecutar delay de 1000 ms (1 segundo) con el valor actual
    delay_software_ms(1000);
    
    // Tomar tiempo final
    uint32_t t_end = drv_tiempo_actual_ms();
    uint32_t duracion_real = t_end - t_start;
    
    // Apagar todos los LEDs de calibración primero
    drv_led_establecer(LED_ENCENDER, LED_OFF);
    drv_led_establecer(LED_APAGAR, LED_OFF);
    
    // Verificar la duración real y encender LEDs correspondientes
    if (duracion_real >= 900 && duracion_real <= 1100) {
        // Duración correcta (~1000ms ±100ms) - encender LED1
        drv_led_establecer(LED_ENCENDER, LED_ON);
    } else if (duracion_real >= 30000) {
        // Duración muy larga (30 segundos o más) - encender LED2
        drv_led_establecer(LED_APAGAR, LED_ON);
    } else {
        // Duración fuera de rangos esperados - apagar ambos
        drv_led_establecer(LED_ENCENDER, LED_OFF);
        drv_led_establecer(LED_APAGAR, LED_OFF);
    }
}

/* ----------------- WRAPPER ----------------- */
bool test_blink_v2(void) {
	
	  drv_leds_iniciar();
    drv_tiempo_iniciar();
	  calibrar_ciclos_por_ms(); 
	
	
    bool ok1 = test_encender_led(LED_ENCENDER);
    bool ok2 = test_apagar_led(LED_APAGAR);
    bool ok3 = test_conmutar_led(LED_CONMUTAR);
    bool ok4 = test_esperar_ms(TIEMPO_ESPERA_MS);

    drv_led_establecer(LED_ENCENDER, ok1 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_APAGAR, ok2 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_CONMUTAR, ok3 ? LED_ON : LED_OFF);
    drv_led_establecer(LED_TEST_OK, ok4 ? LED_ON : LED_OFF);

    while (1) { 
        // Mantener estado para inspección
    }

    //return (ok1 && ok2 && ok3 && ok4);
}
