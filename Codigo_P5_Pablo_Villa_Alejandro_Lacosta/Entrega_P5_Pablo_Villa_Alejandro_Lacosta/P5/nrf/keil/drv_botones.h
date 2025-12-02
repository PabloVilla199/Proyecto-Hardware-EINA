/**
 * @file drv_botones.h
 * @brief Driver independiente del hardware para la gestión de botones con antirrebotes.
 *
 * Este módulo implementa una máquina de estados (FSM) que gestiona los distintos
 * estados de una pulsación de botón: reposo, rebote, muestreo y salida.
 *
 * Se apoya en los módulos:
 *   - hal_ext_int: para recibir interrupciones externas al pulsar un botón
 *   - hal_gpio: para leer el estado físico del pin
 *   - svc_alarmas: para temporizar retardos y muestreos
 *   - rt_fifo: para encolar eventos (opcional)
 */

#ifndef DRV_BOTONES_H
#define DRV_BOTONES_H

#include <stdint.h>
#include "hal_gpio.h"
#include "hal_ext_int.h"
#include "svc_alarmas.h"
#include "rt_fifo.h"
#include "rt_evento.h"
#include "svc_GE.h"


#define TRP 200   
#define TEP 20    
#define TRD 50   
#define NUM_BOTONES 4

typedef enum {
    E_ESPERANDO = 0,  
    E_REBOTES,        
    E_MUESTREO,      
    E_SALIDA       
} estado_boton_t;


typedef struct {
    HAL_GPIO_PIN_T   pin;
    hal_ext_int_id_t id_int;
    estado_boton_t   estado;
} boton_t;

/**
 * @brief Inicializa el módulo de botones.
 *        Configura los GPIO, las interrupciones externas y deja la FSM en reposo.
 */
void drv_botones_iniciar(void(*cb_a_llamar), EVENTO_T ev1_pulsar, EVENTO_T ev2_tiempo);

/**
 * @brief Función llamada por el HAL (callback) al producirse una interrupción externa.
 * @param id Identificador de la interrupción externa (botón pulsado)
 */
void drv_botones_callback(hal_ext_int_id_t id);

/**
 * @brief Actualiza el estado de la FSM de botones ante un evento recibido.
 * @param ev Tipo de evento (EV_PULSACION o EV_TIEMPO)
 * @param id ID del botón afectado
 */
void drv_botones_actualizar(EVENTO_T ev, hal_ext_int_id_t aux);

#endif /* DRV_BOTONES_H */
