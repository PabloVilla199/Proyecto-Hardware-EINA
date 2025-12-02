/* *****************************************************************************
 * P.H.2025: rt_GE.c
 * 
 * Gestor de Eventos (GE) del Runtime del sistema embebido.
 *
 * Coordina:
 *  - Suscripciones (svc_GE)
 *  - Alarmas periódicas e inactividad (svc_alarmas)
 *  - Despacho de eventos a tareas suscritas
 *  - Gestión de bajo consumo (hal_consumo_dormir)
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 * *****************************************************************************/

#include "rt_GE.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "rt_FIFO.h"
#include "hal_consumo.h"
#include "rt_fifo.h"


#define TIEMPO_INACTIVIDAD_MS 5000 

// -----------------------------------------------------------------------------
// Variables internas estáticas
// -----------------------------------------------------------------------------
static uint32_t s_M_overflow = 0;   // Monitor de overflow
static bool s_inicializado = false;

// -----------------------------------------------------------------------------
// Inicializa estructuras y suscripciones base del sistema
// -----------------------------------------------------------------------------
void rt_GE_iniciar(uint32_t M_overflow) {
    s_M_overflow = M_overflow;
    rt_FIFO_inicializar(M_overflow);
	
	  svc_alarma_iniciar(M_overflow, rt_FIFO_encolar, ev_T_PERIODICO);
    svc_GE_suscribir(ev_INACTIVIDAD, 2, rt_GE_actualizar);
}

/* ============================
 * LANZADOR PRINCIPAL
 * ============================ */
/**
 * @brief Bucle principal del gestor de eventos.
 * 
 * Activa la alarma de inactividad, extrae eventos de la FIFO y los despacha.
 * Si no hay eventos pendientes, entra en modo de bajo consumo.
 */
void rt_GE_lanzador(void) {
    EVENTO_T id_evento;
    uint32_t aux_data;
    Tiempo_us_t tiempo;

    // Activar alarma de inactividad (ejemplo: 5 segundos)
    uint32_t flags_inactividad = svc_alarma_codificar(false, TIEMPO_INACTIVIDAD_MS, 0);
    svc_alarma_activar(flags_inactividad, ev_INACTIVIDAD, 0);

    while (1) {
        // Si hay evento, procesarlo
        if (rt_FIFO_extraer(&id_evento, &aux_data, &tiempo)) {
            // Despachar a las tareas suscritas (tabla SVC_GE)
					
					if (id_evento == ev_T_PERIODICO) {
                svc_alarma_actualizar(id_evento, aux_data);
            }
					
            for (int i = 0; i < rt_GE_MAX_SUSCRITOS; i++) {
                if (s_tabla[i].activa && s_tabla[i].evento == id_evento) {
                    s_tabla[i].f_callback(id_evento, aux_data);
                }
            }

            // Reprogramar la alarma de inactividad
            rt_GE_actualizar(id_evento, aux_data);
        } 
        else {
            // Si no hay eventos, esperar
            drv_consumo_esperar();
        }
    }
}

void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t aux) {
	
    uint32_t flags_inactividad;  // ? visible en toda la función

    if (ID_evento == ev_INACTIVIDAD) {
        hal_consumo_dormir();
        return; // el sistema se apaga, no seguimos
    } 
    else {
        // Reprogramar la alarma de inactividad (5s, no periódica)
        flags_inactividad = svc_alarma_codificar(false, TIEMPO_INACTIVIDAD_MS, 0);
        svc_alarma_activar(flags_inactividad, ev_INACTIVIDAD, 0);
    }
}