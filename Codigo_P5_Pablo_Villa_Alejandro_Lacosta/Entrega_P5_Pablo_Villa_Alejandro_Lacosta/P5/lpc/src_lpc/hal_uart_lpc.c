/* *****************************************************************************
 * P.H.2025: hal_uart_lpc.c
 * HAL para UART1 en LPC2105
 *
 * Configura UART1:
 *  - Pines: P0.8 -> TXD1, P0.9 -> RXD1
 *  - Velocidad configurable (default 115200 baudios)
 *  - Formato: 8N1
 *  - FIFO habilitado y reiniciado
 *
 * Funciones básicas de inicialización y envío de caracteres.
 * *****************************************************************************/

#include <LPC210x.H>
#include "hal_uart.h"
#include <stdint.h>

#ifndef PCLK_HZ
#define PCLK_HZ 15000000u            /* PCLK = 15 MHz */
#endif

#ifndef UART_BAUD
#define UART_BAUD 115200u            /* Baudrate por defecto */
#endif

/**
 * @brief Configura los pines P0.8 y P0.9 para UART1 (TXD1/RXD1)
 */
static inline void uart1_set_pins(void) {
    PINSEL0 &= ~(3u << 16);
    PINSEL0 |=  (1u << 16);  /* P0.8 -> TXD1 */
    PINSEL0 &= ~(3u << 18);
    PINSEL0 |=  (1u << 18);  /* P0.9 -> RXD1 */
}

/**
 * @brief Configura la velocidad de la UART1.
 * @param baud Baudios deseados
 */
static inline void uart1_set_baud(uint32_t baud) {
    if (baud == 0) baud = UART_BAUD;
    uint32_t div = (PCLK_HZ + (8u * baud)) / (16u * baud);
    if (div == 0) div = 1;

    U1LCR |= 0x80;                /* DLAB=1 para escribir DLL/DLM */
    U1DLL  = (uint8_t)(div & 0xFF);
    U1DLM  = (uint8_t)((div >> 8) & 0xFF);
    U1LCR &= ~0x80;               /* DLAB=0 */
}

/**
 * @brief Inicializa la UART1 con configuración 8N1 y FIFO habilitado.
 */
void hal_uart_init(void) {
    uart1_set_pins();

    U1LCR = 0x83;                 /* 8N1 + DLAB=1 */
    uart1_set_baud(UART_BAUD);

    U1FCR = 0x07;                 /* FIFO enable + reset RX/TX */
    U1LCR = 0x03;                 /* 8N1, DLAB=0 */
}

/**
 * @brief Envía un carácter por UART1.
 *
 * Convierte '\n' en '\r\n' automáticamente.
 *
 * @param ch Carácter a enviar
 * @return 0 siempre
 */
int hal_uart_sendchar(char ch) {
    /* Espera a que THR esté vacío (LSR bit5 = THRE) */
    while ((U1LSR & (1u << 5)) == 0) { }

    /* Convertir \n -> \r\n */
    if (ch == '\n') {
        U1THR = '\r';
        while ((U1LSR & (1u << 5)) == 0) { }
    }

    U1THR = (uint8_t)ch;
    return 0;
}
