/******************************************************************************
 * Fichero: svc_alarmas.c
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Servicio de gestión de alarmas temporizadas, tanto periódicas como de
 *   disparo único. Permite registrar alarmas, activarlas, desactivarlas
 *   y notificar su expiración mediante callbacks.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "svc_alarmas.h"
#include "drv_tiempo.h"
#include "drv_uart.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "svc_GE.h"
#include "rt_fifo.h"

#define RETARDO_PERIODICO   250

typedef struct {
    bool activa;
    bool periodica;
    uint8_t flags;
    uint32_t retardo_ms;  
    uint32_t comienzo_ms;
    EVENTO_T ID_evento;
    uint32_t auxData;
} ALARMA_T;

static ALARMA_T alarmas[SVC_ALARMAS_MAX];
static SVC_ALARMA_CALLBACK_T func_callback = NULL;
static EVENTO_T evento_tick;
static uint32_t monitor_overflow = 0;


static ALARMA_T* buscar_alarma(EVENTO_T ID_evento, uint32_t auxData) {
    for (int i = 0; i < SVC_ALARMAS_MAX; i++)
        if (alarmas[i].activa && alarmas[i].ID_evento == ID_evento && alarmas[i].auxData == auxData)
            return &alarmas[i];
    return NULL;
}

static ALARMA_T* buscar_libre(void) {
    for (int i = 0; i < SVC_ALARMAS_MAX; i++)
        if (!alarmas[i].activa)
            return &alarmas[i];
    return NULL;
}

void svc_alarma_desactivar(EVENTO_T ID_evento, uint32_t auxData) {
    ALARMA_T *a = buscar_alarma(ID_evento, auxData);
    if (a) a->activa = false;
}

static void tick_handler(void) {
     rt_FIFO_encolar(evento_tick, 0);
}



void svc_alarma_iniciar(uint32_t M_overflow, SVC_ALARMA_CALLBACK_T callback, EVENTO_T ID_evento_tick) {
    func_callback = callback;
    evento_tick = ID_evento_tick;
	  monitor_overflow = M_overflow;
		(void)monitor_overflow;

    for (int i = 0; i < SVC_ALARMAS_MAX; i++) {
        alarmas[i].activa = false;
        alarmas[i].periodica = false;
        alarmas[i].flags = 0;
        alarmas[i].retardo_ms = 0;
        alarmas[i].comienzo_ms = 0;
        alarmas[i].ID_evento = ev_VOID;
        alarmas[i].auxData = 0;
    }
		//svc_GE_suscribir(evento_tick, 1, svc_alarma_actualizar);
    drv_tiempo_periodico_ms(1, tick_handler, evento_tick);
}


void svc_alarma_activar(uint32_t alarma_flags, EVENTO_T ID_evento, uint32_t auxData) {
    bool periodica = (alarma_flags & 0x1);
    uint8_t flags = (alarma_flags >> 1) & 0x7F;
		uint32_t retardo_ms = (alarma_flags >> 8) & 0x00FFFFFF;

    ALARMA_T *alarma = buscar_alarma(ID_evento, auxData);
    if (!alarma) alarma = buscar_libre();
    if (!alarma || retardo_ms == 0) return;

    alarma->activa = true;
    alarma->periodica = periodica;
    alarma->flags = flags;
    alarma->retardo_ms = retardo_ms;           
    alarma->comienzo_ms = drv_tiempo_actual_ms();
    alarma->ID_evento = ID_evento;
    alarma->auxData = auxData;
}

void svc_alarma_actualizar(EVENTO_T ID_evento, uint32_t auxData) {
    if (ID_evento != evento_tick) return;

    uint32_t ahora = drv_tiempo_actual_ms();

    for (int i = 0; i < SVC_ALARMAS_MAX; i++) {
        if (!alarmas[i].activa) continue;

        if ((ahora - alarmas[i].comienzo_ms) >= alarmas[i].retardo_ms) {
            if (func_callback)
                func_callback(alarmas[i].ID_evento, alarmas[i].auxData);

            if (alarmas[i].periodica)
                alarmas[i].comienzo_ms = ahora;
            else
                alarmas[i].activa = false;     
        }
    }
}

uint32_t svc_alarma_codificar(bool periodica, uint32_t retardo_ms, uint8_t flags) {
    uint32_t valor = 0;
    valor |= (periodica ? 1 : 0);
    valor |= ((flags & 0x7F) << 1);
    valor |= (retardo_ms & 0x00FFFFFF) << 8; 
    return valor;
}
