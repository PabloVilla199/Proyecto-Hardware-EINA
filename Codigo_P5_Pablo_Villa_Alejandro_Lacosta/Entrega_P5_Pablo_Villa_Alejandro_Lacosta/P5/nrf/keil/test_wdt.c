#include "test_wdt.h"
#include "drv_wdt.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "svc_logs.h"

#define TIEMPO_WDT_SEG  2   // El perro muerde a los 2 segundos
#define CICLOS_VIVO     5   // Cuantas veces lo alimentamos antes de morir

void test_wdt_suicidio(void) {
    LOG_INFO("=== INICIO TEST WATCHDOG ===");
    LOG_INFO("Configurando WDT a 2 segundos...");

    // 1. Iniciar el Watchdog
    drv_wdt_iniciar(TIEMPO_WDT_SEG);

    // 2. Fase de supervivencia (Alimentar)
    LOG_INFO("Fase 1: Alimentando al perro (El sistema NO debe reiniciarse)");
    
    for (int i = 0; i < CICLOS_VIVO; i++) {
        // Feedback visual
        drv_led_conmutar((LED_id_t)1); 
        
        LOG_INFO("Alimentando WDT... (Sistema OK)");
        
        // ALIMENTAR AL PERRO
        drv_wdt_alimentar();
        
        // Esperar 1 segundo (menos que el timeout de 2s)
        drv_tiempo_esperar_ms(1000);
    }

    // 3. Fase de muerte (Simular cuelgue)
    LOG_INFO("Fase 2: Dejo de alimentar (Simulando bloqueo...)");
    LOG_INFO("Esperando RESET en 2 segundos...");
    
    // Encendemos un LED rojo (o el 4) fijo para indicar pánico
    drv_led_establecer((LED_id_t)4, LED_ON);

    // BUCLE INFINITO SIN ALIMENTAR
    // Aquí simulamos que el procesador se ha quedado colgado
    while(1) {
        // No hacemos nada. El contador del WDT sigue bajando...
        // Tick... tock...
    }
    
    // El código NUNCA debería llegar aquí
    LOG_ERROR("ERROR: El Watchdog no ha funcionado. Sigo vivo.");
}