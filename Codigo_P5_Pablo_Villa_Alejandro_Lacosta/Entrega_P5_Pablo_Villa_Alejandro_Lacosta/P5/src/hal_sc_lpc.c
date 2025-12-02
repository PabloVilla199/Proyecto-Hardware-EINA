#include <LPC210x.H>
#include "hal_sc.h"

static uint32_t sc_nesting = 0;

uint32_t hal_sc_entrar(void) {
    __disable_irq();      // no preserva estado previo
    return ++sc_nesting;
}
void hal_sc_salir(void) {
    if (sc_nesting && --sc_nesting == 0) {
        __enable_irq();   // puede habilitar IRQs aunque estuvieran deshabilitadas antes
    }
}
