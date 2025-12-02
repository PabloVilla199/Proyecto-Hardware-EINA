/******************************************************************************
 * Fichero: drv_uart.h
 * Proyecto: P.H.2025
 *
 * Implementacion del DRV de LOGS
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/
 

/**
 * Inicia el controlador del uart
 */
void drv_uart_init(void);

/**
 * Escribe un mensaje por la linea serie.
 */
void drv_uart_send(const char* message);
