/* *****************************************************************************
 * P.H.2025: GPIOs en LPC2105, Timer 0 y Timer 1
 * Implementación para cumplir el hal_gpio.h
 * Las interrupciones externas para botones se gestionan en otro módulo
 * *****************************************************************************/

#include <LPC210x.H>
#include "board_lpc.h" 
#include "hal_gpio.h"

/**
 * @brief Inicializa el módulo GPIO.
 *
 * Configura todos los pines como entrada por seguridad para evitar cortocircuitos.
 * Configura también los LEDs definidos en LEDS_LIST como salida y en estado OFF.
 */
void hal_gpio_iniciar(void)
{
    IOCLR = 0xFFFFFFFFu;  /* Apagar todo inicialmente */

#if defined(LEDS_NUMBER) && (LEDS_NUMBER > 0)
    static const unsigned char leds[] = LEDS_LIST;  /* Ejemplo: {3,4,5,6} */
    for (unsigned i = 0; i < LEDS_NUMBER; ++i) {
        unsigned pin = leds[i];

        /* Forzar función GPIO=00 SOLO en ese pin */
        if (pin < 16u) PINSEL0 &= ~(3u << (pin*2));
        else           PINSEL1 &= ~(3u << ((pin-16u)*2));

        IODIR |= (1u << pin);  /* Configurar como salida */

        /* Inicializar estado OFF según lógica activa-alta o activa-baja */
        if (LEDS_ACTIVE_STATE) IOCLR = (1u << pin);
        else                   IOSET = (1u << pin);
    }
#endif
}

/* *****************************************************************************
 * Acceso a los GPIOs
 * *****************************************************************************/

/**
 * @brief Configura la dirección del GPIO como entrada o salida.
 * @param gpio Pin a configurar
 * @param direccion Dirección: HAL_GPIO_PIN_DIR_INPUT o HAL_GPIO_PIN_DIR_OUTPUT
 */
void hal_gpio_sentido(HAL_GPIO_PIN_T gpio, hal_gpio_pin_dir_t direccion)
{
    uint32_t masc = (1UL << gpio);
    if (direccion == HAL_GPIO_PIN_DIR_INPUT) {
        IODIR &= ~masc;  /* Entrada */
    } else if (direccion == HAL_GPIO_PIN_DIR_OUTPUT) {
        IODIR |= masc;   /* Salida */
    }       
}

/**
 * @brief Lee el valor lógico del GPIO.
 * @param gpio Pin a leer
 * @return 0 si está bajo, 1 si está alto
 */
uint32_t hal_gpio_leer(HAL_GPIO_PIN_T gpio)
{
    uint32_t masc = (1UL << gpio);
    return ((IOPIN & masc) != 0);
}

/**
 * @brief Escribe un valor en el GPIO.
 * @param gpio Pin a escribir
 * @param valor 0 para bajo, cualquier otro valor para alto
 */
void hal_gpio_escribir(HAL_GPIO_PIN_T gpio, uint32_t valor)
{
    uint32_t masc = (1UL << gpio);
    if ((valor & 0x01) == 0) IOCLR = masc;  /* Poner bajo */
    else                     IOSET = masc;  /* Poner alto */
}
