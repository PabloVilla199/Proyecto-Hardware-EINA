/**
 * @file drv_botones.c
 * @brief Implementación del driver de botones con control de rebotes y FSM.
 */

#include "drv_botones.h"
#include <stdbool.h>
#include <stdio.h>

static boton_t botones[NUM_BOTONES];

static void programar_alarma_unica(uint32_t retardo_ms, hal_ext_int_id_t id);
static void programar_alarma_periodica(uint32_t periodo_ms, hal_ext_int_id_t id);
static bool boton_soltado(hal_ext_int_id_t id);


void drv_botones_callback(hal_ext_int_id_t id){
    hal_ext_int_deshabilitar(id);
    rt_FIFO_encolar(ev_PULSAR_BOTON, (uint32_t) id);
}

static void drv_botones_cb(EVENTO_T ev, uint32_t aux)
{
    hal_ext_int_id_t id = (hal_ext_int_id_t)aux;

    switch (ev) {
        case ev_PULSAR_BOTON:   // viene del HAL (IRQ)
            drv_botones_actualizar(ev_PULSAR_BOTON, id);
            break;
        case ev_BOTON_RETARDO:  // viene del SVC alarmas
            drv_botones_actualizar(ev_T_PERIODICO, id);
            break;
        default:
            /* ignorar otros eventos */
            break;
    }
}

void drv_botones_iniciar(void(*cb_a_llamar), EVENTO_T ev1_pulsar, EVENTO_T ev2_pulsar){
    hal_gpio_iniciar();
    for (uint8_t i = 0; i < NUM_BOTONES; i++) {
			  hal_ext_int_id_t id = i;
        botones[i].pin = i;          
        botones[i].id_int = id;       
        botones[i].estado = E_ESPERANDO;
        hal_gpio_sentido(botones[i].pin, HAL_GPIO_PIN_DIR_INPUT);
    }
    hal_ext_int_iniciar(drv_botones_callback);
		
    for (uint8_t i = 0; i < NUM_BOTONES; i++) {
        hal_ext_int_habilitar(botones[i].id_int);
    }

		svc_GE_suscribir(ev1_pulsar, 0, drv_botones_cb);
		svc_GE_suscribir(ev2_pulsar, 1, drv_botones_cb);
			
}

void drv_botones_actualizar(EVENTO_T ev, hal_ext_int_id_t aux){
    hal_ext_int_id_t id = (hal_ext_int_id_t)aux;
    boton_t *b = &botones[id];

    switch (b->estado) {
    case E_ESPERANDO:
        if (ev == ev_PULSAR_BOTON) {
            b->estado = E_REBOTES;
            programar_alarma_unica(TRP, id);       // anti-rebote de presión
        }
        break;

    case E_REBOTES:
        if (ev == ev_BOTON_RETARDO) {
            b->estado = E_MUESTREO;
            programar_alarma_periodica(TEP, id);   // encuesta periódica
        }
        break;

    case E_MUESTREO:
        if (ev == ev_BOTON_RETARDO) {
            if (boton_soltado(id)) {
                b->estado = E_SALIDA;
                programar_alarma_unica(TRD, id);   // anti-rebote de depresión
            } else {
                // sigue pulsado: reprogramamos TEP
                programar_alarma_periodica(TEP, id);
            }
        }
        break;

    case E_SALIDA:
        if (ev == ev_BOTON_RETARDO) {
            b->estado = E_ESPERANDO;
            hal_ext_int_habilitar(id);             // rearmar la IRQ del botón
        }
        break;
    }
}

static void programar_alarma_unica(uint32_t retardo_ms, hal_ext_int_id_t id)
{
    uint32_t cod = svc_alarma_codificar(false, retardo_ms, 0);
	  svc_alarma_activar(cod, ev_PULSAR_BOTON, id);
}

static void programar_alarma_periodica(uint32_t periodo_ms, hal_ext_int_id_t id)
{
    uint32_t cod = svc_alarma_codificar(true, periodo_ms, 0);
    svc_alarma_activar(cod, ev_PULSAR_BOTON, id);
}

static bool boton_soltado(hal_ext_int_id_t id)
{
    return hal_gpio_leer(botones[id].pin) != 0; // 1 = sin pulsar, 0 = pulsado
}
