/* *****************************************************************************
 * P.H.2025: rt_GE.c
 * Gestor de Eventos (GE) del Runtime del sistema embebido.
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 * *****************************************************************************/

#include "rt_GE.h"
#include <stdint.h>
#include "rt_evento.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "rt_FIFO.h"
#include "hal_consumo.h"
#include "drv_wdt.h"


extern Suscripcion_t s_tabla[rt_GE_MAX_SUSCRITOS];

#define TIEMPO_INACTIVIDAD_MS 10000u 

static uint32_t s_M_overflow = 0;   // Monitor de overflow (Guardado para uso interno)
static bool s_inicializado = false; // Flag de protecci?n contra reinicializaci?n


void rt_GE_iniciar(uint32_t M_overflow) {
   
    if (s_inicializado) {
        return;
    }
    s_inicializado = true; 
    s_M_overflow = M_overflow;

    rt_FIFO_inicializar(s_M_overflow); 
    svc_alarma_iniciar(s_M_overflow, (SVC_ALARMA_CALLBACK_T)rt_FIFO_encolar, ev_T_PERIODICO);
    svc_GE_suscribir(ev_INACTIVIDAD, 2, rt_GE_actualizar);
}



void rt_GE_lanzador(void) {
    EVENTO_T id_evento;
    uint32_t aux_data;
    Tiempo_us_t tiempo;

    // Cadencia de alimentaci?n del WDT
    static uint32_t t_last_feed_ms = 0;
    const  uint32_t FEED_MS = 800;  

    // Alarma de inactividad inicial
    uint32_t flags_inactividad = svc_alarma_codificar(false, TIEMPO_INACTIVIDAD_MS, 0);
    svc_alarma_activar(flags_inactividad, ev_INACTIVIDAD, 0);

    while (1) {
        if (rt_FIFO_extraer(&id_evento, &aux_data, &tiempo)) {

            if (id_evento == ev_T_PERIODICO) {
                svc_alarma_actualizar(id_evento, aux_data);
            }

            for (int i = 0; i < rt_GE_MAX_SUSCRITOS; i++) {
                if (s_tabla[i].activa && s_tabla[i].evento == id_evento) {
                    s_tabla[i].f_callback(id_evento, aux_data);
                }
            }

            rt_GE_actualizar(id_evento, aux_data);

            uint32_t now = drv_tiempo_actual_ms();
            if ((now - t_last_feed_ms) >= FEED_MS) {
                drv_wdt_alimentar();
                t_last_feed_ms = now;
            }

        } else {
            drv_consumo_esperar();
        }
    }
}

void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t aux) {
    if (ID_evento == ev_INACTIVIDAD) {
        drv_wdt_alimentar();
			
        hal_consumo_dormir();
        uint32_t flags = svc_alarma_codificar(false, TIEMPO_INACTIVIDAD_MS, 0);
        svc_alarma_activar(flags, ev_INACTIVIDAD, 0);

        drv_wdt_alimentar();
        return;
    }

    if (ID_evento == ev_PULSAR_BOTON && aux < 4u) {
        uint32_t flags = svc_alarma_codificar(false, TIEMPO_INACTIVIDAD_MS, 0);
        svc_alarma_activar(flags, ev_INACTIVIDAD, 0);
    }
}
