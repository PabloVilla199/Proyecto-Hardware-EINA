/******************************************************************************

* Fichero: svc_GE.h
* Proyecto: P.H. 2025
*
* Descripción:
* Servicio de gestor de eventos (GE). Permite suscribir callbacks a eventos,
* gestionando prioridades y ejecución ordenada.
*
* Autores:
* Alejandro Lacosta
* Pablo Villa
*
* Universidad de Zaragoza
*

*****************************************************************************/

#ifndef SVC_GE_H
#define SVC_GE_H

#include "rt_evento.h"

#define rt_GE_MAX_SUSCRITOS 8

typedef void (*SVC_CALLBACK_T)(EVENTO_T, uint32_t);

typedef struct {
	bool activa;           /**< Indica si esta entrada está en uso */
	EVENTO_T evento;       /**< Evento al que está suscrita */
	uint8_t prioridad;     /**< Prioridad (0 = más alta) */
	SVC_CALLBACK_T f_callback; /**< Función callback asociada al evento */
}Suscripcion_t;

extern Suscripcion_t s_tabla[rt_GE_MAX_SUSCRITOS];

/**

* @brief Suscribe una función callback a un evento con una prioridad específica.
* @param ID_evento Evento al que se suscribe la función.
* @param prioridad Prioridad de la suscripción (0 = más alta).
* @param f_callback Puntero a la función callback.
  */
  void svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, SVC_CALLBACK_T f_callback);

/**

* @brief Cancela una suscripción existente de un evento y compacta la lista de suscripciones.
* @param ID_evento Evento cuya suscripción se desea eliminar.
* @param f_callback Puntero a la función callback que se desea desuscribir.
  */
  void svc_GE_cancelar(EVENTO_T ID_evento, SVC_CALLBACK_T f_callback);

#endif  // SVC_GE_H
