/******************************************************************************
 * Fichero: svc_logs.c
 * Proyecto: P.H.2025
 *
 * Implementacion del Servicio de LOGS
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "svc_logs.h"

/**
 * Inicia el servicio de logs, internamente llama a la funcion
 * iniciar del modulo uart.
 */
void svc_logs_iniciar(void) {
    drv_uart_init(); // Inicializa UART a trav�s de la capa de abstracci�n
}
