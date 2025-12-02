/******************************************************************************
 * Fichero: svc_alarmas.h
 * Proyecto: P.H.2025
 *
 * Módulo de gestión de alarmas del sistema embebido.
 * Permite programar alarmas periódicas o únicas, activar, reprogramar
 * y actualizar alarmas asociadas a eventos.
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef SVC_ALARMAS_H
#define SVC_ALARMAS_H

#include <stdint.h>
#include <stdbool.h>
#include "rt_evento.h"

/**
 * @brief Número máximo de alarmas activas simultáneamente.
 *        Se puede ajustar en compilación según necesidad.
 */
#define SVC_ALARMAS_MAX 8

/**
 * @brief Tipo de callback de alarma.
 * @param ID_evento El identificador del evento asociado a la alarma
 * @param auxData  Datos auxiliares que se pasan al disparar la alarma
 */
typedef void (*SVC_ALARMA_CALLBACK_T)(EVENTO_T ID_evento, uint32_t auxData);

/**
 * @brief Inicializa el módulo de alarmas.
 * @param monitor_overflow Monitor a marcar en caso de overflow
 * @param funcion_callback Callback que se llamará cada tick periódico para gestionar las alarmas
 * @param ID_evento_tick Evento al que se suscribirá el callback periódico (normalmente ev_T_PERIODICO)
 */
void svc_alarma_iniciar(uint32_t monitor_overflow,
                        SVC_ALARMA_CALLBACK_T funcion_callback,
                        EVENTO_T ID_evento_tick);

/**
 * @brief Activa o reprograma una alarma.
 * @param alarma_flags Codificación de la alarma:
 *        - Bit 0: 1 = periódica, 0 = única
 *        - Bits 1-7: flags reservados
 *        - Bits 8-31: retardo en milisegundos
 * @param ID_evento Evento que se disparará al vencer la alarma
 * @param auxData Datos auxiliares que se pasarán al disparar la alarma
 *
 * Si retardo = 0, se desactiva la alarma.
 */
void svc_alarma_activar(uint32_t alarma_flags,
                        EVENTO_T ID_evento,
                        uint32_t auxData);

/**
 * @brief Revisa y dispara las alarmas vencidas.
 * @param ID_evento Evento recibido del tick periódico
 * @param auxData Datos auxiliares del evento (si aplica)
 *
 * Debe llamarse periódicamente, normalmente desde el tick del temporizador.
 */
void svc_alarma_actualizar(EVENTO_T ID_evento, uint32_t auxData);

/**
 * @brief Codifica los parámetros de la alarma en un solo valor de 32 bits.
 * @param periodica true si la alarma debe ser periódica
 * @param retardo_ms Retardo en milisegundos
 * @param flags_flags adicionales (7 bits) reservados para uso futuro
 * @return Valor codificado de 32 bits que representa la alarma
 */
uint32_t svc_alarma_codificar(bool periodica,
                              uint32_t retardo_ms,
                              uint8_t flags);

#endif // SVC_ALARMAS_H
