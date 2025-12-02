/** *****************************************************************************
 * P.H.2025: hal_gpio_nrf.c
 * HAL GPIO para nRF52840 DK 
 *
 * Autores: Alejandro Lacosta, Pablo Villa 
 *
 * Notas generales:
 *  - Los LEDs de la placa estan en modo activos en bajo definido en board_nrf52940dk.h -> #define LEDS_ACTIVE_STATE 0 
 *       valor = 0 ? LED encendido (nivel físico bajo, 0V)
 *       valor = 1 ? LED apagado  (nivel físico alto, 3.3V)
 *
 *  - En los registros del periférico GPIO (NRF_P0):
 *       * OUTSET: el pin pasa a nivel alto (3.3V) =  el LED se apaga
 *       * OUTCLR: el pin pasa a nivel bajo (0V) = el LED se enciende
 *       * DIRSET: pone el pin como salida (output)
 *       * DIRCLR: pone el pin como entrada (input)
 *
 *  - El operador 1UL 
 *  - `asegura que solo se use el bit menos significativo,
 *       por si se pasa un número distinto de 0 o 1.
 *
 * -----------------------------------------------------------------------------
 * FUNCIONES DEL HAL:
 *
 *  ? hal_gpio_iniciar():
 *       Inicializa el subsistema GPIO.
 *       Pone todos los pines como entrada al principio,
 *       y los LEDs los configura como salida y apagados (nivel alto = OFF).
 *
 *  ? hal_gpio_sentido():
 *       Cambia la dirección (sentido) de un pin.
 *       Si lo ponemos a **INPUT (DIRCLR)**, decimos que queremos leer datos
 *       Si lo ponemos a **OUTPUT (DIRSET)**, decimos que queremos enviar datos
 *
 *  ? hal_gpio_leer():
 *       Devuelve el estado lógico del pin (0 o 1).
 *       Si el LED es activo-bajo, OUT=0 significa LED encendido.
 *		   Si el LED es activo-bajo, OUT=1 significa LED Apagado.
 *
 *  ? hal_gpio_escribir():
 *       Escribe un valor lógico (0 o 1) en el pin configurado como salida.
 *       Si escribimos 0 ? usamos OUTCLR  el pin baja a 0V ? LED encendido.
 *       Si escribimos 1 ? usamos OUTSET ?el pin sube a 3.3V ? LED apagado.
 * ----------------------------------------------------------------------------- */

#include "hal_gpio.h"
#include "board_nrf52840dk.h"
#include <nrf.h>


/* Lista de LEDs de la placa */
static const uint32_t led_list[] = LEDS_LIST;

/**
 * @brief Inicializa el subsistema GPIO.
 *
 * - LEDs como salida y apagados según LEDS_ACTIVE_STATE.
 */
void hal_gpio_iniciar(void) {
    // Todos los pines a la DIR = 0 
    NRF_P0->DIR = 0x00000000;

    // Inicializar LEDs configurandolos a la salida y apagandolos 
    for (int i = 0; i < LEDS_NUMBER; i++) {
        hal_gpio_sentido(led_list[i], HAL_GPIO_PIN_DIR_OUTPUT);
        hal_gpio_escribir(led_list[i], 1); 
    }
}

/**
 * @brief Configura la dirección (sentido) de un pin GPIO
 *
 * - DIRCLR: pone el pin como (INPUT)
 *           queremos leer datos del pin
 * - DIRSET: pone el pin como (OUTPUT)
 *           queremos escribir datos en al pin
 *
 * @param gpio     Número del pin GPIO
 * @param direccion Dirección deseada (INPUT/OUTPUT)
 */

void hal_gpio_sentido(HAL_GPIO_PIN_T gpio, hal_gpio_pin_dir_t direccion) {
    if (direccion == HAL_GPIO_PIN_DIR_INPUT) {
        NRF_P0->DIRCLR = (1UL << gpio);
    } else {
        NRF_P0->DIRSET = (1UL << gpio);
    }
}

/**
 * @brief Lee valor lógico de un pin (0 = OFF lógico, 1 = ON lógico)
 */
uint32_t hal_gpio_leer(HAL_GPIO_PIN_T gpio) {
    uint32_t nivel = (NRF_P0->OUT & (1UL << gpio)) ? 1 : 0;

    return (nivel == 0) ? 0 : 1;
}

/**
 * @brief Escribe valor lógico de un pin:
 *        valor = 1 ? LED ON  (OUTCLR, nivel físico 0)
 *        valor = 0 ? LED OFF (OUTSET, nivel físico 1)
 */
void hal_gpio_escribir(HAL_GPIO_PIN_T gpio, uint32_t valor) {
    valor &= 1UL; // asegurar solo 0 o 1

    if (valor == 0) {
        // LED ON ? nivel físico bajo
        NRF_P0->OUTCLR = (1UL << gpio);
    } else {
        // LED OFF ? nivel físico alto
        NRF_P0->OUTSET = (1UL << gpio);
    }
}
