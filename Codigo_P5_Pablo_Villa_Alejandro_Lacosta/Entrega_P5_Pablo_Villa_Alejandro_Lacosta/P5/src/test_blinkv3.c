/* *****************************************************************************
 * P.H.2025: test_blink_v3.c
 *
 * Pruebas funcionales del módulo blink3
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Comprueba la correcta operación del blink_v3:
 * 
 * Los resultados se muestran mediante LEDs de estado.
 *
 ******************************************************************************/

#include "test_blink_v3.h"

#define LED_TEST_OK       1
#define LED_TEST_FAIL     2
#define PERIODO_MS        100
#define MARGEN_ERROR_MS   20
#define NUM_CALLBACKS     3  // Probar varios ciclos

// Variables globales para el test
static volatile bool callback_llamado = false;
static volatile uint32_t t_callback_inicio = 0;
static volatile uint32_t t_callback_fin = 0;
static volatile uint32_t contador_callbacks = 0;
static volatile uint32_t tiempos_callbacks[NUM_CALLBACKS];

// ------------------ FUNCIONES DEL TEST ------------------

// Callback del timer periódico
static void test_callback(void) {
    if (contador_callbacks == 0) {
        t_callback_inicio = drv_tiempo_actual_ms();
    }
    
    t_callback_fin = drv_tiempo_actual_ms();
    
    if (contador_callbacks < NUM_CALLBACKS) {
        tiempos_callbacks[contador_callbacks] = t_callback_fin;
    }
    
    contador_callbacks++;
    callback_llamado = true;
}

// Inicializa el test
static void test_iniciar(void) {
    drv_leds_iniciar();
    drv_tiempo_iniciar();
    callback_llamado = false;
    contador_callbacks = 0;
    t_callback_inicio = 0;
    t_callback_fin = 0;
    
    for (int i = 0; i < NUM_CALLBACKS; i++) {
        tiempos_callbacks[i] = 0;
    }
}

// Arranca el temporizador periódico
static void test_arrancar_periodico(uint32_t periodo_ms) {
    test_iniciar();
    drv_tiempo_periodico_ms(periodo_ms, test_callback, 1);
}

// Espera a que se ejecute el callback
static void test_esperar_callback(void) {
    uint32_t timeout = drv_tiempo_actual_ms() + PERIODO_MS * (NUM_CALLBACKS + 2);
    
    while (contador_callbacks < NUM_CALLBACKS) {
        if (drv_tiempo_actual_ms() > timeout) {
            break; // Timeout para evitar bloqueo infinito
        }
    }
}

// Comprueba si los callbacks se ejecutaron dentro del margen
static bool test_verificar_tiempos(uint32_t esperado_ms, uint32_t margen_ms) {
    // Verificar primer callback
    if (contador_callbacks == 0) {
        return false;
    }
    
    // Verificar periodicidad de todos los callbacks
    for (uint32_t i = 1; i < contador_callbacks && i < NUM_CALLBACKS; i++) {
        uint32_t periodo_medido = tiempos_callbacks[i] - tiempos_callbacks[i-1];
        int32_t error = (int32_t)periodo_medido - (int32_t)esperado_ms;
        
        if (error < -(int32_t)margen_ms || error > (int32_t)margen_ms) {
            return false;
        }
    }
    
    return true;
}

// Verificar que se llamó al callback
static bool test_verificar_callback_llamado(void) {
    return (contador_callbacks > 0);
}

// Indica resultado con LEDs
static void test_resultado(bool ok) {
    if (ok) {
        drv_led_establecer(LED_TEST_OK, LED_ON);
        drv_led_establecer(LED_TEST_FAIL, LED_OFF);
    } else {
        drv_led_establecer(LED_TEST_OK, LED_OFF);
        drv_led_establecer(LED_TEST_FAIL, LED_ON);
    }
}

// ------------------ MAIN ------------------
bool test_blink_v3(void) {
    test_iniciar();
    
    // Test 1: Verificar que el callback se llama
    test_arrancar_periodico(PERIODO_MS);
    test_esperar_callback();
    
    bool test1_ok = test_verificar_callback_llamado();
    
    // Test 2: Verificar periodicidad
    bool test2_ok = test_verificar_tiempos(PERIODO_MS, MARGEN_ERROR_MS);
    
    // Resultado final (ambos tests deben pasar)
    bool final_ok = test1_ok && test2_ok;
    
    test_resultado(final_ok);
    
    while (1) {
        // Mantener estado para inspección
        // El callback debería seguir ejecutándose periódicamente
    }
    
   // return 0;
}
