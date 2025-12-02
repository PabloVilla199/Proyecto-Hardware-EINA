/******************************************************************************
 * Fichero: svc_logs.c
 * Proyecto: P.H.2025
 *
 * Descripcion de Endpoints del Servicio de LOGS
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/


#ifndef SVC_LOGS_H
#define SVC_LOGS_H

#include <stdint.h>
#include "drv_uart.h"


// === Definición de niveles de log ===
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_INFO    2
#define LOG_LEVEL_DEBUG   3
#define LOG_LEVEL_NONE    0

// === Configuración del nivel actual de log ===
// Cambiar aquí para ajustar el nivel de detalle de salida
#define LOG_LEVEL LOG_LEVEL_DEBUG

// === Inicialización del servicio de logs ===
void svc_logs_iniciar(void);

// === Macros de log condicional ===
#define ENDLINE "\r\n"

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    #define LOG_DEBUG(msg) drv_uart_send("DEBUG: " msg ENDLINE)
#else
    #define LOG_DEBUG(msg)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
    #define LOG_INFO(msg) drv_uart_send("INFO: " msg ENDLINE)
#else
    #define LOG_INFO(msg)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
    #define LOG_ERROR(msg) drv_uart_send("ERROR: " msg ENDLINE)
#else
    #define LOG_ERROR(msg)
#endif

#endif // SVC_LOGS_H
