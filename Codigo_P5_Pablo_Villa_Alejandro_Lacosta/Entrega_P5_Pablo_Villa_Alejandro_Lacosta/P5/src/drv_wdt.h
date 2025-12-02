/******************************************************************************
 * Fichero: drv_wdt.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Driver del Watchdog Timer. Proporciona una interfaz sencilla para su
 *   inicialización y alimentación. La operación de alimentación se realiza
 *   dentro de una sección crítica para garantizar atomicidad.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef DRV_WDT_H
#define DRV_WDT_H

#include <stdint.h>

/**
 * Inicializa el Watchdog con el tiempo límite especificado.
 * El parámetro indica el número de unidades de tiempo antes del reinicio.
 */
void drv_wdt_iniciar(uint32_t sec);

/**
 * Alimenta el Watchdog.
 * Esta operación se realiza dentro de una sección crítica para que la
 * secuencia de alimentación no pueda ser interrumpida.
 */
void drv_wdt_alimentar(void);

#endif /* DRV_WDT_H */

