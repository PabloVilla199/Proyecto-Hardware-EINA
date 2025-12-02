/******************************************************************************
 * Fichero: rt_evento.h
 * Proyecto: P.H.2025
 *
 * Definici�n de tipos y estructuras de eventos para el Runtime del sistema embebido.
 * Incluye enumeraci�n de eventos, tipos de usuario y estructura base de evento.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef RT_EVENTO_H
#define RT_EVENTO_H

#include <stdint.h>
#include "drv_tiempo.h"


typedef enum {
    ev_VOID = 0,          // sin evento
    ev_T_PERIODICO = 1,   // tick del temporizador peri�dico
    ev_PULSAR_BOTON = 2,  // ejemplo: evento externo (bot�n, sensor, etc.)
	  ev_BOTON_RETARDO = 3,
	  ev_INACTIVIDAD = 4,  // no existe actividad 
	  ev_BEAT_TIMEOUT = 5,
} EVENTO_T;

#define EVENT_TYPES 6  // n�mero total de tipos de evento
#define ev_NUM_EV_USUARIO 1
#define ev_USUARIO {ev_PULSAR_BOTON,  ev_BOTON_RETARDo}
#define ev_TIMER {ev_PERIODICO}


typedef struct {
    EVENTO_T    tipo;       // ID o tipo del evento
    uint32_t    origen;     // fuente del evento (opcional)
    uint32_t    auxData;    // dato auxiliar (valor, par�metro, etc.)
    Tiempo_us_t ts;         // marca de tiempo en microsegundos
    uint8_t     prioridad;  // prioridad opcional (0=normal, 1=alta)
} rt_evento_t;

#endif // RT_EVENTO_H
