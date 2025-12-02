/******************************************************************************
 * Fichero: hal_wdt.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Interfaz hardware del Watchdog Timer. Define las funciones de bajo nivel
 *   para configurar el WDT y para alimentarlo periódicamente desde el software.
 *
 * Autores:
 * Alejandro Lacosta 
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>

/**
 * @brief Configura e inicia el Watchdog Timer.
 * @param timeout_ms Tiempo en milisegundos antes de provocar un reinicio
 *                   si no es alimentado.
 */
void hal_wdt_iniciar(uint32_t timeout_ms);

/**
 * @brief Alimenta al Watchdog (reinicia su contador de cuenta atrás).
 * Debe llamarse periódicamente para evitar que expire.
 */
void hal_wdt_alimentar(void);

#endif
