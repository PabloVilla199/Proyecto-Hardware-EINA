#include "hal_sc.h"
#include "nrf.h"  

static uint32_t m_in_critical_region = 0;

uint32_t hal_sc_entrar(void) {
    __disable_irq();
    return ++m_in_critical_region;
}

void hal_sc_salir(void) {
    if (m_in_critical_region > 0)
        m_in_critical_region--;
    if (m_in_critical_region == 0)
        __enable_irq();
}
