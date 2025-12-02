#ifndef HAL_SC_H
#define HAL_SC_H

#include <stdint.h>

/**
 * @brief Entra en sección crítica (deshabilita interrupciones)
 * @return Nivel de anidamiento actual
 */
uint32_t hal_sc_entrar(void);

/**
 * @brief Sale de sección crítica (rehabilita interrupciones si corresponde)
 */
void hal_sc_salir(void);

#endif // HAL_SC_H
