#ifndef HAL_EXT_INT_H
#define HAL_EXT_INT_H

#include <stdint.h>

typedef enum {
    HAL_EXT_INT_0,
    HAL_EXT_INT_1,
    HAL_EXT_INT_2,
    HAL_EXT_INT_3,
} hal_ext_int_id_t;

// Tipo de función callback a ejecutar cuando se produzca una interrupción
typedef void (*hal_ext_int_callback_t)(hal_ext_int_id_t id);

// Inicializa el sistema de interrupciones externas
void hal_ext_int_iniciar(hal_ext_int_callback_t cb);

// Habilita la interrupción externa correspondiente a un pin/botón
void hal_ext_int_habilitar(hal_ext_int_id_t id);

// Deshabilita la interrupción externa correspondiente
void hal_ext_int_deshabilitar(hal_ext_int_id_t id);

// Configura qué interrupciones pueden despertar al sistema del modo bajo consumo
void hal_ext_int_habilitar_despertar(hal_ext_int_id_t id);
void hal_ext_int_deshabilitar_despertar(hal_ext_int_id_t id);

#endif // HAL_EXT_INT_H
