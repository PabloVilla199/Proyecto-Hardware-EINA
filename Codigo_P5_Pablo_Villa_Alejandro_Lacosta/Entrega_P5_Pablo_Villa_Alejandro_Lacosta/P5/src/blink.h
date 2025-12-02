/******************************************************************************
 * Fichero: blink.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Módulo encargado de seleccionar y ejecutar las distintas versiones del
 *   programa Blink, así como los tests asociados cuando el modo TESTING está
 *   habilitado. Centraliza la lógica de despacho según la sesión indicada.
 *
 * Autores:
 * Alejandro Lacosta 
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef BLINK_H
#define BLINK_H


#include <stdint.h>
#include <stdbool.h>
#include "hal_gpio.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "drv_uart.h"
#include "svc_logs.h"
#include "rt_fifo.h"

/**
 * @brief Ejecuta la versión de Blink o el test correspondiente según la sesión.
 *
 * @param sesion  ID de la sesión (1, 2, 3, 4, 5, 6)
 * @param debug   true para ejecutar los tests, false para ejecutar las versiones normales (blink_vX)
 */
void blink_ejecutar(uint8_t sesion);

#endif // BLINK_H
