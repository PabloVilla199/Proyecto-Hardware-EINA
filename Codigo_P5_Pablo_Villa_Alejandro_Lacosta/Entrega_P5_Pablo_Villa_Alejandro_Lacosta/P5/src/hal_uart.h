/******************************************************************************
 * Fichero: svc_logs.c
 * Proyecto: P.H.2025
 *
 * Descrpcion de Endpoints de hal de LOGS
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef HAL_UART_H
#define HAL_UART_H

// Inicializa la UART espec�fica de cada plataforma
void hal_uart_init(void);

// Env�a un caracter a trav�s de la UART
int hal_uart_sendchar(char ch);

#endif // HAL_UART_H
