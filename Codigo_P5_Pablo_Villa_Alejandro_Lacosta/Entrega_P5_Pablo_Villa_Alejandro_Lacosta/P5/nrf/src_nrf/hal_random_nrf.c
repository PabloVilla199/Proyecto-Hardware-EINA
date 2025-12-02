/**
 * *****************************************************************************
 * P.H.2025: hal_wdt_nrf.c
 * Implementación del HAL del Random para nRF52840
 *
 * Autores: Alejandro Lacosta y Pablo Villa
 * Universidad de Zaragoza
 * ****************************************************************************/

#include <stdint.h>
#include "nrf.h"

/*
 * El RNG hardware del nRF no usa semilla. Produce bytes aleatorios a partir de
 * ruido f?sico.
 */
void hal_random_iniciar(uint32_t seed) {

    (void)seed;

    // Activa el "digital error correction" del RNG (recomendado por Nordic)
    NRF_RNG->CONFIG = (RNG_CONFIG_DERCEN_Enabled << RNG_CONFIG_DERCEN_Pos);
    NRF_RNG->EVENTS_VALRDY = 0;
    NRF_RNG->TASKS_START = 1;
}

/*
 * Devuelve un n?mero aleatorio entre 1 y max (incluidos).
 *
 * - Espera a que el hardware indique que hay un byte aleatorio disponible.
 * - Lee ese byte de NRF_RNG->VALUE (0?255).
 * - Lo reduce al rango 1..max usando un m?dulo.
 */
uint32_t hal_random(uint32_t max) {
    if (max == 0)
        return 0;  

    while (NRF_RNG->EVENTS_VALRDY == 0);
    NRF_RNG->EVENTS_VALRDY = 0;

    uint32_t rand_value = NRF_RNG->VALUE;
    return (rand_value % max) + 1;
}
