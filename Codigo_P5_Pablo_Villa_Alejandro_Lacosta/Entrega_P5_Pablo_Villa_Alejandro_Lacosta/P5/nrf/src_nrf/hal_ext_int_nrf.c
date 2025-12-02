/**
 * *****************************************************************************
 * P.H.2025: hal_ext_int_nrf.c
 * HAL de interrupciones externas para nRF52840 (Gen�rico)
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * - Los pines est�n configurados con resistencias pull-up internas.
 * Estado l�gico:
 * valor = 1 -> Nivel alto (No activo)
 * valor = 0 -> Nivel bajo (Activo / Disparado)
 *
 * - El callback registrado por el usuario se ejecuta cuando ocurre una 
 * interrupci�n en cualquiera de los pines definidos.
 * * *****************************************************************************
 */

#include "hal_ext_int.h"
#include "nrf.h"
#include <stdbool.h>

#define EXT_INT_NUMBER  4

static const uint32_t pines_irq[EXT_INT_NUMBER] = {11, 12, 24, 25};

static hal_ext_int_callback_t cb_usuario = 0;

/**
 * @brief Rutina de servicio de interrupciones (ISR) del perif�rico GPIOTE.
 *
 * Se ejecuta autom�ticamente cuando ocurre una interrupci�n externa
 */
void GPIOTE_IRQHandler(void)
{
    for (uint8_t i = 0; i < EXT_INT_NUMBER; i++) {
        if (NRF_GPIOTE->EVENTS_IN[i]) {
            NRF_GPIOTE->EVENTS_IN[i] = 0; // Limpiar bandera
            
            hal_ext_int_deshabilitar((hal_ext_int_id_t)i);
            
            if (cb_usuario) {
                cb_usuario((hal_ext_int_id_t)i);
            }
        }
    }
    NRF_GPIOTE->EVENTS_PORT = 0;
}

/**
 * @brief Inicializa el sistema de interrupciones externas (GPIOTE).
 */
void hal_ext_int_iniciar(hal_ext_int_callback_t cb)
{
    cb_usuario = cb;

    // 1. Configurar los pines como entrada con Pull-Up
    for (uint8_t i = 0; i < EXT_INT_NUMBER; i++) {
        uint32_t pin = pines_irq[i];
        
        NRF_P0->PIN_CNF[pin & 0x1F] =
            (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos) | 
            (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) | 
            (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    }

    for (uint8_t i = 0; i < EXT_INT_NUMBER; i++) {
        uint32_t pin = pines_irq[i];

        NRF_GPIOTE->CONFIG[i] =
            (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) | 
            (pin << GPIOTE_CONFIG_PSEL_Pos) | 
            (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);
            
        NRF_GPIOTE->EVENTS_IN[i] = 0;
    }

    NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN0_Set << 0) |
                           (GPIOTE_INTENSET_IN1_Set << 1) |
                           (GPIOTE_INTENSET_IN2_Set << 2) |
                           (GPIOTE_INTENSET_IN3_Set << 3);
        
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

/**
 * @brief Habilita la interrupci�n externa correspondiente a un pin/canal.
 *
 * @param id  Identificador de la interrupci�n (0 a EXT_INT_NUMBER - 1)
 */
void hal_ext_int_habilitar(hal_ext_int_id_t id){
    if (id < EXT_INT_NUMBER) {
        NRF_GPIOTE->INTENSET = (1 << (GPIOTE_INTENSET_IN0_Pos + id));
        NRF_GPIOTE->EVENTS_IN[id] = 0;
        NVIC_EnableIRQ(GPIOTE_IRQn);
    }
}

/**
 * @brief Deshabilita la interrupci�n externa correspondiente a un pin/canal.
 *
 * @param id  Identificador de la interrupci�n (0 a EXT_INT_NUMBER - 1)
 */
void hal_ext_int_deshabilitar(hal_ext_int_id_t id){
    if (id < EXT_INT_NUMBER) {
        NRF_GPIOTE->INTENCLR = (1 << (GPIOTE_INTENCLR_IN0_Pos + id));
    }
}

/**
 * @brief Habilita la detecci�n por nivel bajo para permitir el despertar del MCU.
 *
 * @param id  Identificador de la interrupci�n (0 a EXT_INT_NUMBER - 1)
 *
 * Esta configuraci�n permite que el microcontrolador salga del modo 
 * bajo consumo cuando se detecte una se�al de nivel bajo.
 */
void hal_ext_int_habilitar_despertar(hal_ext_int_id_t id){
    if (id < EXT_INT_NUMBER) {
        uint32_t pin = pines_irq[id];
        NRF_P0->PIN_CNF[pin & 0x1F] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
    }
}

/**
 * @brief Deshabilita la detecci�n por nivel bajo en un pin.
 *
 * @param id  Identificador de la interrupci�n (0 a EXT_INT_NUMBER - 1)
 */
void hal_ext_int_deshabilitar_despertar(hal_ext_int_id_t id){
    if (id < EXT_INT_NUMBER) {
        uint32_t pin = pines_irq[id];
        NRF_P0->PIN_CNF[pin & 0x1F] &= ~(GPIO_PIN_CNF_SENSE_Msk);
    }
}

/**
 * @brief Lee el estado actual del pin f�sico.
 * @param pin N�mero de pin f�sico (0-31 del Puerto 0)
 */
uint32_t hal_ext_int_leer_pin(uint8_t pin) {
    return (NRF_P0->IN >> pin) & 0x01;
}
