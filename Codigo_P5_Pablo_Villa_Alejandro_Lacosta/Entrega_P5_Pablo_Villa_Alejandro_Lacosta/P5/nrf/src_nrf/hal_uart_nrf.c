/* *****************************************************************************
 * P.H.2025: hal_tiempo_nrf.c
 * HAL de temporizador para nRF52840 DK
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Implementa:
 *  - Reloj mon�tono de 24-bit (SysTick)
 *  - Reloj peri�dico con callback (RTC1)
 *
 * Notas:
 *  - SysTick: alta precisi�n (usado para medir tiempo en �s o ms)
 *  - RTC1: bajo consumo, usado para temporizaci�n peri�dica (32768 Hz)
 *  - Prescaler = 0 ? frecuencia base del RTC = 32768 Hz
 *
 ******************************************************************************/

#include <nrf.h>
#include "hal_uart.h"

#define PIN_TXD (6)
#define PIN_RXD (8)

void hal_uart_init() {
    // Configura el pin TXD como salida
    NRF_GPIO->PIN_CNF[PIN_TXD] = 
        (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    // Configura el pin RXD como entrada
    NRF_GPIO->PIN_CNF[PIN_RXD] = 
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    // Configura la UART en el NRF52840
    NRF_UARTE0->CONFIG = (UARTE_CONFIG_HWFC_Disabled << UARTE_CONFIG_HWFC_Pos) |
                         (UARTE_CONFIG_PARITY_Excluded << UARTE_CONFIG_PARITY_Pos);
    NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud115200;
    NRF_UARTE0->PSEL.TXD = PIN_TXD;
    NRF_UARTE0->PSEL.RXD = PIN_RXD;
    NRF_UARTE0->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
}

int hal_uart_sendchar(char ch) {
	// Apuntar al car�cter y definir su tama�o para UART
	NRF_UARTE0->TXD.PTR = (uint32_t)&ch;
	NRF_UARTE0->TXD.MAXCNT = 1;  // Solo un car�cter

	NRF_UARTE0->EVENTS_ENDTX = 0;
	NRF_UARTE0->TASKS_STARTTX = 1;

	while (NRF_UARTE0->EVENTS_ENDTX == 0);
	
	return 0;
}
