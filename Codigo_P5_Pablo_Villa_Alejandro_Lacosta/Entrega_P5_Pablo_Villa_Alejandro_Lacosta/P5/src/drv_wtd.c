/******************************************************************************
 * Fichero: drv_wdt.c
 * Proyecto: P.H. 2025
 *
 * Implementación del driver de alto nivel para el Watchdog Timer.
 * Se encarga de inicializar y alimentar el WDT utilizando la capa HAL
 * correspondiente.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "drv_wdt.h"
#include "hal_wdt.h"

void drv_wdt_iniciar(uint32_t sec) {
    hal_wdt_iniciar(sec * 1000);
}

void drv_wdt_alimentar(void) {
    hal_wdt_alimentar();
}
