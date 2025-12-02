/* *****************************************************************************
 * P.H.2025: hal_consumo_nrf.c
 *
 * HAL de gesti?n de bajo consumo
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Implementa funciones para poner el micro en espera o dormir.
 * dependiendo de la situaci?n, reduciendo el consumo energ?tico.
 *
 * Notas:
 *   - Se utiliza __WFI() (Wait For Interrupt) para modo espera ligero.
 *   - Para dormir completamente se utiliza SYSTEMOFF y __WFE()
 *
 ******************************************************************************/ 
 
#include <nrf.h>
#include "hal_consumo.h"

/* Inicializa el m?dulo de consumo */
void hal_consumo_iniciar(void) {}

/* Pone el micro en modo espera ligero (__WFI) hasta la siguiente interrupci?n */
void hal_consumo_esperar(void) {
    __WFI();
}

/* Pone el micro en modo sue?o profundo (SYSTEMOFF), funcion no retorna */
void hal_consumo_dormir(void) {	
	// Evento que ha de suceder para que despierte
	NRF_POWER->SYSTEMOFF = POWER_SYSTEMOFF_SYSTEMOFF_Enter;
	//solo sale por reset o SENSE GPIO si esta programado
	while (1){
		__WFE(); // para los eventos del debug!!
	}
}
