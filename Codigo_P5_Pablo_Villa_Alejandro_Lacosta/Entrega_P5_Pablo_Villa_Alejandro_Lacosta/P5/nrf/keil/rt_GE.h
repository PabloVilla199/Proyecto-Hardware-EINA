#ifndef RT_GE_H
#define RT_GE_H

#include <stdint.h>
#include "rt_evento.h"
#include "svc_GE.h"

// Inicializa las estructuras estaticas y suscribe los eventos de usuario 
void rt_GE_iniciar(uint32_t M_overflow);

// Activa la alarma de inactividad "ev_INACTIVIDAD", y si hay eventos --> los despacha a sus respectivas tareas suscritas, 
// si no -->  espera 
void rt_GE_lanzador(void);

// Si ocurre un evento de usuario --> reprograma la alarma, si ocurre evento de inactividad --> deepSleep()
void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t aux);


#endif
