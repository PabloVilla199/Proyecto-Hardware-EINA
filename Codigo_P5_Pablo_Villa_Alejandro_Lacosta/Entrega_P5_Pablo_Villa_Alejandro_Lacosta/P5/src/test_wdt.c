/******************************************************************************
 * Fichero: test_wdt.c
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Implementación de la prueba del Watchdog Timer. El test mantiene el
 *   watchdog alimentado durante varios ciclos para asegurar que el sistema
 *   no se reinicia. Después, deja de alimentarlo para simular un cuelgue,
 *   provocando que el WDT fuerce un reinicio automático.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "test_wdt.h"
#include "drv_wdt.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "svc_logs.h"

#define TIEMPO_WDT_SEG  2   // El perro muerde a los 2 segundos
#define CICLOS_VIVO     5   // Cuantas veces lo alimentamos antes de morir

/**
 * @brief Ejecuta la prueba "suicida" del Watchdog.
 *
 * Mantiene el WDT alimentado durante varios ciclos mientras muestra
 * información por los logs y parpadea un LED. Después deja de alimentarlo
 * deliberadamente para provocar un reinicio del sistema.
 */
void test_wdt_suicidio(void) {
    LOG_INFO("=== INICIO TEST WATCHDOG ===");
    LOG_INFO("Configurando WDT a 2 segundos...");

    drv_wdt_iniciar(TIEMPO_WDT_SEG);

    LOG_INFO("Fase 1: Alimentando al perro (El sistema NO debe reiniciarse)");
    
    for (int i = 0; i < CICLOS_VIVO; i++) {
        drv_led_conmutar((LED_id_t)1); 
        LOG_INFO("Alimentando WDT... (Sistema OK)");
        drv_wdt_alimentar();
        drv_tiempo_esperar_ms(1000);
    }

    LOG_INFO("Fase 2: Dejo de alimentar (Simulando bloqueo...)");
    LOG_INFO("Esperando RESET en 2 segundos...");
    
    drv_led_establecer((LED_id_t)4, LED_ON);

    while(1) {
        // Simulación de cuelgue: no alimentar el WDT
    }
}
