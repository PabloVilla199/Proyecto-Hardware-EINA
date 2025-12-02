/******************************************************************************
 * Fichero: drv_botones.h
 * Proyecto: P.H.2025
 *
 * Driver/Manejador de botones (depuraci�n)
 *
 * Este m�dulo permite comunicarte con los botones para su pulsacion
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/
 
#ifndef DRV_BOTONES_H
#define DRV_BOTONES_H

#include "hal_gpio.h"
#include "hal_ext_int.h"
#include <stdbool.h>

// ===============================
// Configuraci�n de tiempos (ms)
// ===============================

#define TRP 200    // Tiempo de rebote de presi�n
#define TEP 20     // Tiempo de encuesta peri�dica
#define TRD 50     // Tiempo de rebote de depresi�n


// ===============================
// Definici�n de estados FSM
// ===============================
typedef enum {
    E_ESPERANDO = 0,  ///< Espera de pulsaci�n
    E_REBOTES,        ///< Espera fin de rebote de presi�n
    E_MUESTREO,       ///< Muestreo peri�dico mientras est� pulsado
    E_SALIDA          ///< Espera fin de rebote de depresi�n
} estado_boton_t;

// ===============================
// Estructura de control del bot�n
// ===============================
typedef struct {
    HAL_GPIO_PIN_T pin;          ///< Pin asociado al bot�n
    hal_ext_int_id_t id_int;     ///< ID de interrupci�n externa
    estado_boton_t estado;       ///< Estado actual del bot�n
} boton_t;

// ===============================
// API del driver
// ===============================
/*
 * @brief Inicializa el m�dulo de botones.
 *        Configura los GPIO, interrupciones externas y FSM en reposo.
 *
 * @param cb_a_llamar Callback a ejecutar ante eventos de bot�n.
 * @param ev1_pulsar  Evento asociado a la pulsaci�n.
 * @param ev2_tiempo  Evento asociado a temporizaci�n.
 */ 

void drv_botones_iniciar(void(*cb_a_llamar),uint32_t ev_pulsar, uint32_t ev_ignorado);
/**
 * @brief Callback del HAL al producirse una interrupci�n externa.
 * @param id Identificador de la interrupci�n externa (bot�n pulsado)
 */ 
void drv_botones_callback(hal_ext_int_id_t id);

/**
 * @brief Actualiza la FSM de un bot�n ante un evento recibido.
 * @param ev  Tipo de evento (pulsaci�n o retardo)
 * @param id  ID del bot�n afectado
 */
void drv_botones_actualizar(uint32_t ev, hal_ext_int_id_t id);

/**
 * @brief Devuelve una cadena con el estado actual del bot�n indicado.
 * @param id ID del bot�n a consultar
 * @return Puntero a cadena descriptiva ("ESPERANDO", "REBOTES", etc.)
 */
const char* drv_botones_estado_str(uint8_t id);


bool drv_boton_esta_pulsado(uint8_t boton_id);

#endif  // DRV_BOTONES_H 
