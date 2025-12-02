#ifndef RT_FIFO_H
#define RT_FIFO_H

#include <stdint.h>
#include "rt_evento.h"

// Tamaño máximo de la cola
#define RT_FIFO_TAM 64

// === Funciones principales ===

// Inicializa la cola de eventos
void rt_FIFO_inicializar(uint32_t monitor_overflow);

// Encola un evento con ID y dato auxiliar
// Internamente añade marca de tiempo (TS en microsegundos)
void rt_FIFO_encolar(uint32_t ID_evento, uint32_t auxData);

// Extrae el evento más antiguo
// Devuelve el número de eventos restantes en cola (0 si está vacía)
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS);

// Devuelve estadísticas de eventos:
//  - Si ID_evento == ev_VOID ? nº total de eventos encolados
//  - Si ID_evento válido ? nº de veces que ese tipo se ha encolado
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento);

bool rt_FIFO_test(void);

#endif // RT_FIFO_H
