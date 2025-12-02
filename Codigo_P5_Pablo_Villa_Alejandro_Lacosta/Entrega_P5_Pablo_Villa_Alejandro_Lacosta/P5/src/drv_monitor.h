/******************************************************************************
 * Fichero: drv_monitor.h
 * Proyecto: P.H.2025
 *
 * Driver/Manejador de MONITORES (depuración)
 *
 * Permite marcar, desmarcar o conmutar pines GPIO para visualización
 * mediante un analizador lógico o medidor de consumo.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef DRV_MONITOR_H
#define DRV_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Inicializa el gestor de monitores, configurando los pines GPIO como salida.
 * @return Número total de monitores disponibles en la placa.
 */
uint32_t drv_monitor_iniciar(void);

/**
 * @brief Marca un monitor (pone el pin correspondiente a nivel alto lógico).
 * @param id Identificador del monitor (1..n)
 */
void drv_monitor_marcar(uint32_t id);

/**
 * @brief Desmarca un monitor (pone el pin correspondiente a nivel bajo lógico).
 * @param id Identificador del monitor (1..n)
 */
void drv_monitor_desmarcar(uint32_t id);


#endif // DRV_MONITOR_H
