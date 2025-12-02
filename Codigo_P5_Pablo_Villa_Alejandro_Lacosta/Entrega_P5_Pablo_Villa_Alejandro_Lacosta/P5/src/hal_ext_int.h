/******************************************************************************
 * Fichero: hal_ext_int.h
 * Proyecto: P.H. 2025
 *
 * Abstracción del hardware de interrupciones externas (External Interrupts)
 * Permite inicializar, habilitar/deshabilitar interrupciones y configurar
 * callbacks asociados a pines o botones.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_EXT_INT_H
#define HAL_EXT_INT_H

#include <stdint.h>

/**
 * @brief Identificadores de interrupciones externas
 */
typedef enum {
    HAL_EXT_INT_0,
    HAL_EXT_INT_1,
    HAL_EXT_INT_2,
    HAL_EXT_INT_3,
} hal_ext_int_id_t;

/**
 * @brief Tipo de función callback a ejecutar cuando se produzca una interrupción
 */
typedef void (*hal_ext_int_callback_t)(hal_ext_int_id_t id);

/**
 * @brief Inicializa el sistema de interrupciones externas
 * @param cb Callback a ejecutar cuando ocurra una interrupción
 */
void hal_ext_int_iniciar(hal_ext_int_callback_t cb);

/**
 * @brief Habilita la interrupción externa indicada
 */
void hal_ext_int_habilitar(hal_ext_int_id_t id);

/**
 * @brief Deshabilita la interrupción externa indicada
 */
void hal_ext_int_deshabilitar(hal_ext_int_id_t id);

/**
 * @brief Lee el estado lógico de un pin de interrupción
 * @param pin Número del pin
 * @return Valor lógico actual del pin (0 o 1)
 */
uint32_t hal_ext_int_leer_pin(uint8_t pin);

/**
 * @brief Configura qué interrupciones pueden despertar al sistema desde bajo consumo
 */
void hal_ext_int_habilitar_despertar(hal_ext_int_id_t id);
void hal_ext_int_deshabilitar_despertar(hal_ext_int_id_t id);

#endif // HAL_EXT_INT_H
