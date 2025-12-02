/* *****************************************************************************
 * P.H.2025: hal_consumo.h
 *
 * HAL de gestión de bajo consumo
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Funciones para poner el micro en espera o dormir.
 *
 ******************************************************************************/ 

#ifndef HAL_CONSUMO_H
#define HAL_CONSUMO_H

#include <stdint.h>
#include <stdbool.h>

/* Inicializa el módulo de consumo*/
void hal_consumo_iniciar(void);

/* Pone el micro en modo espera ligero hasta la siguiente interrupción */
void hal_consumo_esperar(void);

/* Pone el micro en modo sueño profundo  */
void hal_consumo_dormir(void);

#endif /* HAL_CONSUMO_H */
