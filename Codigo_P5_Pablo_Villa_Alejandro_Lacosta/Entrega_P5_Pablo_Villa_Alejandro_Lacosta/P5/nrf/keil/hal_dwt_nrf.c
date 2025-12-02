/**
 * *****************************************************************************
 * P.H.2025: hal_wdt_nrf.c
 * Implementación del HAL del Watchdog Timer para nRF52840
 *
 * Autores: Alejandro Lacosta y Pablo Villa
 * Universidad de Zaragoza
 * ****************************************************************************/

#include "hal_wdt.h"
#include "nrf.h"

// Valor mágico para recargar el WDT (alimentarlo) según hoja técnica nRF52
#define NRF_WDT_RR_VALUE  0x6E524635UL 

/**
 * @brief Configura e inicia el Watchdog Timer.
 * @param timeout_ms El tiempo en milisegundos.
 * * Nota: El WDT del nRF52 funciona con el reloj de 32768 Hz.
 * Fórmula: CRV = (tiempo_segundos * 32768) - 1
 */
void hal_wdt_iniciar(uint32_t timeout_ms) {
    
    // 1. Configurar comportamiento: 
    // El WDT debe seguir contando incluso si la CPU está dormida (Sleep)
    // WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos
    NRF_WDT->CONFIG = (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos) |
                      (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos); 

    // 2. Calcular valor de recarga (CRV)
    // CRV = (ms * 32768) / 1000 - 1.
    // Usamos uint64_t intermedio para evitar desbordamiento en la multiplicación
    uint32_t crv_value = (uint32_t)(((uint64_t)timeout_ms * 32768ULL) / 1000ULL);
    
    NRF_WDT->CRV = crv_value;

    // 3. Habilitar el registro de recarga 0 (RR[0])
    NRF_WDT->RREN = (WDT_RREN_RR0_Enabled << WDT_RREN_RR0_Pos);

    // 4. Iniciar el Watchdog (Task START)
    NRF_WDT->TASKS_START = 1;
}

/**
 * @brief Alimenta al Watchdog.
 * Escribe el valor mágico en el registro de recarga habilitado.
 */
void hal_wdt_alimentar(void) {
    // Recargar el contador usando el registro RR[0]
    NRF_WDT->RR[0] = NRF_WDT_RR_VALUE;
}

void hal_wdt_parar(void){

}
