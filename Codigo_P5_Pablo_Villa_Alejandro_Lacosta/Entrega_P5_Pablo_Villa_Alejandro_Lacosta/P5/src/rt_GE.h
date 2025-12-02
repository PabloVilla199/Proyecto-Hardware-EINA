/******************************************************************************
 * Fichero: rt_ge.h
 * Proyecto: P.H.2025
 *
 * Gestión de eventos (GE) en tiempo real.
 * Inicializa estructuras, suscribe eventos de usuario y maneja la lógica
 * de activación de alarmas y despacho de eventos.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef RT_GE_H
#define RT_GE_H

#include <stdint.h>
#include "rt_evento.h"
#include "svc_GE.h"
#include "drv_consumo.h"
#include "hal_consumo.h"

/**
 * @brief Inicializa las estructuras estáticas y suscribe los eventos de usuario
 * @param M_overflow Tiempo base de overflow para temporización interna
 */
void rt_GE_iniciar(uint32_t M_overflow);

/**
 * @brief Activa la alarma de inactividad ("ev_INACTIVIDAD") y despacha eventos
 * de usuario a sus tareas suscritas. Si no hay eventos pendientes, espera.
 */
void rt_GE_lanzador(void);

/**
 * @brief Actualiza el sistema ante la ocurrencia de un evento
 * @param ID_evento Identificador del evento ocurrido
 * @param aux Parámetro auxiliar o dato adicional del evento
 *
 * Si ocurre un evento de usuario, reprograma la alarma.
 * Si ocurre un evento de inactividad, entra en deepSleep().
 */
void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t aux);

#endif // RT_GE_H
