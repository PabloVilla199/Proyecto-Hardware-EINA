/******************************************************************************
 * Fichero: hal_random.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Interfaz de acceso al generador de números aleatorios hardware.
 *   Proporciona funciones para inicializarlo y obtener valores aleatorios
 *   dentro de un rango específico.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_RANDOM_H
#define HAL_RANDOM_H

#include <stdint.h>

/**
 * @brief Inicializa el generador de números aleatorios del hardware.
 *
 * El parámetro `seed` se mantiene por compatibilidad, aunque no se utiliza
 * en plataformas nRF.
 */
void hal_random_iniciar(uint32_t seed);

/**
 * @brief Devuelve un número aleatorio entre 1 y `max` (incluidos).
 *
 * Si `max` es 0, devuelve 0.
 */
uint32_t hal_random(uint32_t max);

#endif // HAL_RANDOM_H

