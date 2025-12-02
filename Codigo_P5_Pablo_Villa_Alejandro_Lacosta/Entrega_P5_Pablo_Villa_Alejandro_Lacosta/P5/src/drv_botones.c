/******************************************************************************
 * Fichero: drv_botones.c
 * Proyecto: P.H.2025
 *
 * Driver/Manejador de botones (depuración)
 *
 * Este módulo permite comunicarte con los botones para su pulsación
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "drv_botones.h"
#include <stdint.h>
#include "hal_gpio.h"
#include "hal_ext_int.h"
#include "svc_alarmas.h"
#include "rt_fifo.h"
#include "svc_GE.h"
#include <stdbool.h>
#include <stdio.h>
#include "board.h"

static boton_t botones[BUTTONS_NUMBER];
static const uint32_t s_board_button_pins[] = BUTTONS_LIST;
static EVENTO_T s_evento_principal; // Guardamos el ID del evento a usar

static void programar_alarma_unica(uint32_t retardo_ms, hal_ext_int_id_t id);
static void programar_alarma_periodica(uint32_t periodo_ms, hal_ext_int_id_t id);

// -----------------------------------------------------------------------------
// Callback de la interrupci?n Hardware
// -----------------------------------------------------------------------------
void drv_botones_callback(hal_ext_int_id_t id){  
    hal_ext_int_deshabilitar(id);
    rt_FIFO_encolar(s_evento_principal, (uint32_t) id);
}

// -----------------------------------------------------------------------------
// Callback del Gestor de Eventos (Dispatcher)
// -----------------------------------------------------------------------------
static void drv_botones_cb(uint32_t ev, uint32_t aux){
    hal_ext_int_id_t id = (hal_ext_int_id_t)aux;
    drv_botones_actualizar(ev, id);
}

// -----------------------------------------------------------------------------
// Inicializaci?n
// -----------------------------------------------------------------------------
void drv_botones_iniciar(void(*cb_a_llamar),uint32_t ev_pulsar, uint32_t ev_ignorado){
    
    s_evento_principal = ev_pulsar; // Guardamos el evento para usarlo en alarmas
    hal_gpio_iniciar();
    
    for (uint8_t i = 0; i < BUTTONS_NUMBER; i++) {
        botones[i].pin = s_board_button_pins[i];
        botones[i].id_int = (hal_ext_int_id_t) i;
        botones[i].estado = E_ESPERANDO;
        hal_gpio_sentido(botones[i].pin, HAL_GPIO_PIN_DIR_INPUT);
    }
    hal_ext_int_iniciar(drv_botones_callback);
    
    for (uint8_t i = 0; i < BUTTONS_NUMBER; i++) {
        hal_ext_int_habilitar(botones[i].id_int);
    }
    svc_GE_suscribir(s_evento_principal, 0, drv_botones_cb);
    
}

// -----------------------------------------------------------------------------
// M?quina de Estados
// -----------------------------------------------------------------------------
void drv_botones_actualizar(uint32_t ev, hal_ext_int_id_t aux){
    hal_ext_int_id_t id = aux;
    boton_t *b = &botones[id];

    // La l?gica depende EXCLUSIVAMENTE del estado actual
    switch (b->estado) {
        
    case E_ESPERANDO:
        b->estado = E_REBOTES;
        programar_alarma_unica(TRP, id);
        break;

    case E_REBOTES:
        b->estado = E_MUESTREO;
        programar_alarma_periodica(TEP, id);
        break;

    case E_MUESTREO:
        if (!hal_gpio_leer(id)) { 
            b->estado = E_SALIDA;
            programar_alarma_unica(TRD, id);
            
        }else {
            // FALSO POSITIVO o REBOTE: A?n no est? estable o se solt?.
            // No cambiamos de estado, dejamos que la alarma peri?dica salte otra vez.
        }
        break;

    case E_SALIDA:
        b->estado = E_ESPERANDO;        
        hal_ext_int_habilitar(id);
        break;
    }
}

// -----------------------------------------------------------------------------
// Helpers de Alarmas
// -----------------------------------------------------------------------------
static void programar_alarma_unica(uint32_t retardo_ms, hal_ext_int_id_t id){
    uint32_t cod = svc_alarma_codificar(false, retardo_ms, 0);
    svc_alarma_activar(cod, s_evento_principal, id);
}

static void programar_alarma_periodica(uint32_t periodo_ms, hal_ext_int_id_t id){
    uint32_t cod = svc_alarma_codificar(true, periodo_ms, 0);
    svc_alarma_activar(cod, s_evento_principal, id);
}

bool drv_boton_esta_pulsado(uint8_t boton_id) {  
	if(boton_id <= BUTTONS_NUMBER) return false;
	return (hal_gpio_leer(botones[boton_id].pin) == 0);
}


const char* drv_botones_estado_str(uint8_t id) {
    if (id >= BUTTONS_NUMBER) return "INVALIDO";
    switch (botones[id].estado) {
        case E_ESPERANDO: return "ESPERANDO";
        case E_REBOTES:   return "REBOTES";
        case E_MUESTREO:  return "MUESTREO";
        case E_SALIDA:    return "SALIDA";
        default:          return "???";
    }
} 
