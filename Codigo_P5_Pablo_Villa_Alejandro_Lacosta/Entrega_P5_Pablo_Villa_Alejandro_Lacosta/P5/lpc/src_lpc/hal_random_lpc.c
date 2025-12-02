/* *****************************************************************************
 * P.H.2025: hal_random_lpc.c
 * HAL para generación de números aleatorios en LPC2105
 *
 * Generador PRNG xorshift32:
 *  - Estado de 32 bits
 *  - Rápido y seguro para tareas no criptográficas
 *  - Permite inicialización con semilla o con entropía HW (timers + GPIO)
 *
 * Funciones protegidas para su uso desde ISR o threads mediante hal_SC.
 * *****************************************************************************/

#include <LPC210x.H>
#include <stdint.h>
#include "hal_random.h"
#include "hal_SC.h"   // protección del estado si se usa desde thread e ISR

/* Estado del generador (no debe ser 0) */
static volatile uint32_t s_state = 0;

/**
 * @brief Obtiene entropía básica del hardware.
 *
 * Mezcla contadores de timers y GPIO para obtener un valor semialeatorio
 * si no se proporciona semilla.
 *
 * @return Valor de 32 bits pseudoaleatorio
 */
static uint32_t read_hw_jitter(void) {
    uint32_t v = T1TC ^ T0TC ^ IOPIN;
    /* Desordenar bits */
    v ^= (v << 13);
    v ^= (v >> 17);
    v ^= (v << 5);
    return v;
}

/**
 * @brief Paso del xorshift32.
 *
 * Genera el siguiente estado del PRNG.
 *
 * @param x Estado actual
 * @return Nuevo estado
 */
static inline uint32_t xorshift32_step(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

/**
 * @brief Inicializa el generador de números aleatorios.
 *
 * Si seed=0, se obtiene entropía del hardware.
 * Asegura que el estado interno nunca sea cero.
 *
 * @param seed Semilla inicial
 */
void hal_random_iniciar(uint32_t seed) {
    uint32_t s = seed;
    if (s == 0) {
        s = read_hw_jitter();
        if (s == 0) s = 0xA3C59AC3u; // evitar estado nulo
    }
    if (s == 0) s = 1u; // forzar estado no nulo
    s_state = s;
}

/**
 * @brief Devuelve un número aleatorio en [1..max].
 *
 * Protege el acceso al estado interno para que sea seguro desde ISR o threads.
 * Usa multiplicación de 64 bits para mapear uniformemente [0..2^32-1] -> [0..max-1].
 *
 * @param max Valor máximo
 * @return Número aleatorio en [1..max], o 0 si max=0
 */
uint32_t hal_random(uint32_t max) {
    if (max == 0) return 0;

    hal_sc_entrar();            // proteger s_state
    uint32_t x = s_state;
    if (x == 0) x = 1u;         // evitar estado muerto
    x = xorshift32_step(x);
    s_state = x;
    hal_sc_salir();

    /* Mapear uniformemente a [0..max-1] */
    uint32_t r = (uint32_t)(((uint64_t)x * (uint64_t)max) >> 32);
    return r + 1; // devolver en [1..max]
}
