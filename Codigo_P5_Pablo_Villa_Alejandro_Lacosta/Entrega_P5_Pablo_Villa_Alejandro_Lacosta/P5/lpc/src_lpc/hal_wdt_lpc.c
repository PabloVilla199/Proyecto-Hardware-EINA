/* *****************************************************************************
 * P.H.2025: hal_wdt_nrf.c
 * HAL para Watchdog Timer (WDT) en LPC2105
 *
 * Funciones:
 *  - Inicialización con timeout en milisegundos
 *  - Alimentación periódica del WDT
 *  - (Opcional) Lectura de la causa del último reset por WDT
 * *****************************************************************************/

#include "hal_wdt.h"
#include <LPC210x.H>

/* Bits del registro WDMOD */
#define WDEN_BIT     (1u<<0)  /* Habilita WDT */
#define WDRESET_BIT  (1u<<1)  /* Reset al vencer */
#define WDTOF_BIT    (1u<<2)  /* Time-out flag: indica que el último reset fue por WDT */
#define WDINT_BIT    (1u<<3)  /* Interrupción (no usado) */

#ifndef PCLK_HZ
#define PCLK_HZ (15000000u)   /* Clock periférico por defecto */
#endif

/**
 * @brief Alimenta (feed) al WDT con la secuencia requerida.
 *
 * Protege la secuencia con deshabilitación temporal de interrupciones.
 */
static inline void wdt_feed_sequence(void) {
    __disable_irq();          
    WDFEED = 0xAA;
    WDFEED = 0x55;
    __enable_irq();
}

/**
 * @brief Inicializa el WDT con un timeout en milisegundos.
 *
 * @param timeout_ms Tiempo en ms antes de que el WDT provoque reset
 */
void hal_wdt_iniciar(uint32_t timeout_ms) {
    uint32_t ticks = (uint32_t)(((uint64_t)timeout_ms * (PCLK_HZ/4u)) / 1000ull);
    if (ticks == 0u) ticks = 1u;

    WDTC  = ticks;
    WDMOD = WDEN_BIT | WDRESET_BIT;   /* Habilitar WDT + reset al vencer */

    wdt_feed_sequence();               /* Alimentar para arrancar */
}

/**
 * @brief Alimenta el WDT para evitar el reset.
 */
void hal_wdt_alimentar(void) {
    wdt_feed_sequence();
}

/* 
// Opcional: devuelve 1 si el último reset fue por WDT
int hal_wdt_causa(void)
{
    int fue = (WDMOD & WDTOF_BIT) ? 1 : 0;
    if (fue) {
        WDMOD &= ~WDTOF_BIT;  // limpiar flag
    }
    return fue;
} 
*/
