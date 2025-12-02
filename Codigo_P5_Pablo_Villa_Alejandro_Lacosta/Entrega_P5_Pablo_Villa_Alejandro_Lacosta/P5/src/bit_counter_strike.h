/******************************************************************************
 * Fichero: bit_counter_strike.h
 * Proyecto: Beat Hero - Prácticas P.H. 2025
 *
 * Descripción:
 *   Cabecera del módulo principal del juego "Counter Strike". Contiene la
 *   declaración de la función pública de inicialización y el identificador
 *   de evento usado para temporizaciones internas del juego.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef BIT_COUNTER_STRIKE_H
#define BIT_COUNTER_STRIKE_H

#include <stdint.h>
#include "rt_evento.h"

#define ev_JUEGO_TIMEOUT    ((EVENTO_T)5)

/**
 * @brief Inicia el juego Bit Counter-Strike.
 * Secuencia fija de 8 pasos, dificultad creciente.
 */
void bit_counter_strike_iniciar(void);

#endif /* BIT_COUNTER_STRIKE_H */
