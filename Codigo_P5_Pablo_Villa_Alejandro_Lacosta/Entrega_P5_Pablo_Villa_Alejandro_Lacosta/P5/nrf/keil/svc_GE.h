#ifndef SVC_GE_H
#define SVC_GE_H

#include <stdint.h>
#include <stdbool.h>
#include "rt_evento.h"
#include "hal_tiempo.h"

// Número máximo de suscripciones activas (no por evento, sino total)
#define rt_GE_MAX_SUSCRITOS 4

// Tipo de función callback asociada a un evento
typedef void (*SVC_CALLBACK_T)(EVENTO_T, uint32_t);

// Estructura de suscripción
typedef struct {
    bool activa;           // Indica si esta entrada está en uso
    EVENTO_T evento;       // Qué evento escucha
    uint8_t prioridad;     // Prioridad (0 = más alta)
    SVC_CALLBACK_T f_callback; // Función asociada al evento
} Suscripcion_t;

// ?? Declaración global para que rt_GE.c pueda acceder a la tabla
extern Suscripcion_t s_tabla[rt_GE_MAX_SUSCRITOS];

// ===================== Funciones públicas =====================

// Suscribe una función callback a un evento con prioridad
void svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, SVC_CALLBACK_T f_callback);

// Elimina una suscripción existente y compacta la lista
void svc_GE_cancelar(EVENTO_T ID_evento, SVC_CALLBACK_T f_callback);

#endif  // SVC_GE_H