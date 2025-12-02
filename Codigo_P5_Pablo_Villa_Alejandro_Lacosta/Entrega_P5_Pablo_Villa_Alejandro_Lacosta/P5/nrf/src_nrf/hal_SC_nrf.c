/* *****************************************************************************
 * P.H.2025: hal_SC_nrf.c
 *
 * Hal encargado de activar y desactivar interrupciones para la placa NRF 
 *
 * Autores: Alejandro Lacosta y Pablo Villa
 * Universidad de Zaragoza
 * ****************************************************************************/


#include "hal_sc.h"
#include "nrf.h"  

static uint32_t saved_primask = 0;
static uint32_t nesting = 0;

uint32_t hal_sc_entrar(void) {
    uint32_t primask = __get_PRIMASK(); 
    __disable_irq();

    if (nesting == 0) {
        saved_primask = primask;          
    }

    nesting++;
    return nesting;                       
}

void hal_sc_salir(void) {
    if (nesting == 0)
        return;                           
    nesting--;

    if (nesting == 0) {
        __set_PRIMASK(saved_primask);     
    }
}
