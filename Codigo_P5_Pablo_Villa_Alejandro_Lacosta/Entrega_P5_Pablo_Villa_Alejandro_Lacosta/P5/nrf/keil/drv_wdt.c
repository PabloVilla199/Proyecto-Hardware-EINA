/******************************************************************************
 * Fichero: drv_wdt.c
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Implementación del driver de Watchdog. Proporciona una capa de abstracción
 *   sobre el HAL para inicializar y alimentar el temporizador de vigilancia.
 *   Convierte el tiempo en segundos a milisegundos para plataformas nRF.
 *
 * Autores:
 * Alejandro Lacosta 
 * Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "drv_wdt.h"
#include "hal_wdt.h"

/**
 * @brief Inicializa el Watchdog con un tiempo límite dado en segundos.
 *
 * En nRF, el HAL espera milisegundos, por lo que se convierte la unidad.
 */
void drv_wdt_iniciar(uint32_t sec) {
    hal_wdt_iniciar(sec * 1000); 
}

/**
 * @brief Alimenta el Watchdog para evitar el reinicio.
 *
 */
void drv_wdt_alimentar(void) {
    hal_wdt_alimentar();
}

