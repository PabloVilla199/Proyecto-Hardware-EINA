/******************************************************************************
 * Fichero: hal_sc.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Cabecera del módulo encargado de gestionar las secciones críticas.
 *   Proporciona funciones para deshabilitar y restaurar interrupciones con
 *   control de anidamiento.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_SC_H
#define HAL_SC_H

#include <stdint.h>

/**
 * Entra en una sección crítica deshabilitando interrupciones.
 */
uint32_t hal_sc_entrar(void);

/**
 * Sale de una sección crítica.
 * Restaura las interrupciones.
 */
void hal_sc_salir(void);

#endif /* HAL_SC_H */
