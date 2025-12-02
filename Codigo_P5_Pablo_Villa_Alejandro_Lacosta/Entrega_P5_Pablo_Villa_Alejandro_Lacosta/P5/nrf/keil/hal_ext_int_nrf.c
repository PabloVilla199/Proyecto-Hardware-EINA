/**
 * *****************************************************************************
 * P.H.2025: hal_ext_int_nrf.c
 * HAL de interrupciones externas para nRF52840 DK
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 *  - Los botones del DK están conectados con resistencias pull-up internas,
 *    por lo que su estado lógico es:
 *       valor = 1 ? botón no presionado (nivel alto)
 *       valor = 0 ? botón presionado   (nivel bajo)
 *
 *  - El callback registrado por el usuario se ejecuta cuando ocurre una 
 *    interrupción en cualquiera de los botones definidos.
 * 
 *
 * --------------------------------------------------------------------------- */

#include "hal_ext_int.h"
#include "nrf.h"
#include "board_nrf52840dk.h"
#include <stdbool.h>

static hal_ext_int_callback_t cb_usuario = 0;
static const uint32_t botones_pines[BUTTONS_NUMBER] = BUTTONS_LIST;

/**
 * @brief Rutina de servicio de interrupciones (ISR) del periférico GPIOTE.
 *
 * Se ejecuta automáticamente cuando ocurre una interrupción externa
 */
 
void GPIOTE_IRQHandler(void)
{
    for (uint8_t i = 0; i < BUTTONS_NUMBER; i++) {
        uint32_t pin = botones_pines[i];
        if (NRF_GPIOTE->EVENTS_IN[i]) {
            NRF_GPIOTE->EVENTS_IN[i] = 0;
					  hal_ext_int_deshabilitar(pin);
            if (cb_usuario) {
                cb_usuario((hal_ext_int_id_t)i);
            }
        }
    }
    NRF_GPIOTE->EVENTS_PORT = 0;
}

/**
 * @brief Inicializa el sistema de interrupciones externas (GPIOTE).
 *
 */

void hal_ext_int_iniciar(hal_ext_int_callback_t cb)
{
    cb_usuario = cb;
    for (uint8_t i = 0; i < BUTTONS_NUMBER; i++) {
        uint32_t pin = botones_pines[i];
			
        NRF_P0->PIN_CNF[pin & 0x1F] =
            (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos)| 
            (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)| 
            (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    }
    for (uint8_t i = 0; i < BUTTONS_NUMBER; i++) {
        uint32_t pin = botones_pines[i];

        NRF_GPIOTE->CONFIG[i] =
            (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)| 
            (pin << GPIOTE_CONFIG_PSEL_Pos)| 
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
 * @brief Habilita la interrupción externa correspondiente a un botón.
 *
 * @param id  Identificador del botón (0 a BUTTONS_NUMBER - 1)
 */

void hal_ext_int_habilitar(hal_ext_int_id_t id){
	NRF_GPIOTE->INTENSET = (1 << (GPIOTE_INTENSET_IN0_Pos + id));
	NRF_GPIOTE->EVENTS_IN[id] = 0;
	NVIC_EnableIRQ(GPIOTE_IRQn);
}

/**
 * @brief Deshabilita la interrupción externa correspondiente a un botón.
 *
 * @param id  Identificador del botón (0 a BUTTONS_NUMBER - 1)
 */

void hal_ext_int_deshabilitar(hal_ext_int_id_t id){
  NRF_GPIOTE->INTENCLR = (1 << (GPIOTE_INTENCLR_IN0_Pos + id));
	
}

/*
 * @brief Habilita la detección por nivel bajo para permitir el despertar del MCU.
 *
 * @param id  Identificador del botón (0 a BUTTONS_NUMBER - 1)
 *
 * Esta configuración permite que el microcontrolador salga del modo 
 * bajo consumo cuando se detecte una pulsación (nivel bajo).
 */

void hal_ext_int_habilitar_despertar(hal_ext_int_id_t id){
	uint32_t pin = botones_pines[id];
  NRF_P0->PIN_CNF[pin & 0x1F] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
}

/**
 * @brief Deshabilita la detección por nivel bajo en un pin.
 *
 * @param id  Identificador del botón (0 a BUTTONS_NUMBER - 1)
 */

void hal_ext_int_deshabilitar_despertar(hal_ext_int_id_t id){
	uint32_t pin = botones_pines[id];
  NRF_P0->PIN_CNF[pin & 0x1F] &= ~(GPIO_PIN_CNF_SENSE_Msk);
}
