/******************************************************************************
 * Fichero: hal_wdt.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Abstracción de bajo nivel para el Watchdog Timer. Permite configurarlo
 *   con un tiempo de expiración y alimentarlo de forma periódica desde las
 *   capas superiores del sistema.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>

/**
 * Configura e inicia el Watchdog Timer.
 * El parámetro indica el tiempo en milisegundos antes del reinicio
 * si no se alimenta a tiempo.
 */
void hal_wdt_iniciar(uint32_t timeout_ms);

/**
 * Alimenta el Watchdog (reinicia su contador).
 * Debe invocarse periódicamente desde el bucle principal o desde
 * un punto seguro del sistema.
 */
void hal_wdt_alimentar(void);

#endif /* HAL_WDT_H */

