/******************************************************************************
 * Fichero: drv_uart.c
 * Proyecto: P.H.2025
 *
 * Implementacion del Drv de LOGS
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/
 
#include "hal_uart.h"
#include <stddef.h>

/**
 * Inicia el controlador del uart
 */
void drv_uart_init(void) {
    hal_uart_init();
}

/**
 * Escribe un mensaje por la linea serie.
 */
void drv_uart_send(const char *msg) {
    if (msg == NULL) return;

    while (*msg != '\0') {         
			hal_uart_sendchar(*msg++); // Enviar cada car?cter con hal_uart_sendchar
    }
}
