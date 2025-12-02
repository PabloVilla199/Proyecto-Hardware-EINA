/* *****************************************************************************
 * P.H.2025: hal_consumo_lpc.c
 * HAL de bajo consumo para LPC2105 (ARM7TDMI)
 *
 * Modos soportados:
 *  - Idle: la CPU se detiene; los periféricos continúan funcionando.
 *          Se despierta con cualquier interrupción habilitada.
 *
 *  - Power-down: se detienen oscilador y relojes. Solo despierta por EINTx/RTC
 *                según EXTWAKE. Tras despertar es necesario reenganchar el PLL.
 * *****************************************************************************/

#include <LPC210x.H>
#include "hal_consumo.h"

extern void switch_to_PLL(void);

/**
 * @brief Inicializa la HAL de consumo.
 *
 * No es obligatorio configurar nada aquí. Opcionalmente se pueden deshabilitar
 * periféricos no usados para reducir consumo modificando PCONP.
 */
void hal_consumo_iniciar(void) {
}

/**
 * @brief Entra en modo Idle.
 *
 * La CPU se detiene tras ejecutar esta instrucción.  
 * Los periféricos continúan funcionando.  
 * Se sale del idle con cualquier interrupción habilitada.
 */
void hal_consumo_esperar(void) {
    PCON = 0x01;   /* IDL = 1 → Idle */
    __asm { nop }
    __asm { nop }
}

/**
 * @brief Entra en modo Power-down.
 *
 * Secuencia según manual del LPC2105:
 *  1. Configurar fuentes de wake-up en EXTWAKE.
 *  2. Limpiar flags pendientes en EXTINT.
 *  3. Escribir PCON.PD=1 para entrar en power-down.
 *  4. Al despertar, el hardware estabiliza el oscilador; si se usa PLL,
 *     debe reengancharse con switch_to_PLL().
 *  5. Limpieza de flags de EINT para siguientes entradas en PD.
 */
void hal_consumo_dormir(void) {
    /* Habilitar EINT0, EINT1 y EINT2 como fuentes de wake */
    EXTWAKE |= (1u<<0) | (1u<<1) | (1u<<2);

    /* Limpiar flags pendientes (requisito para entrar en Power-down) */
    EXTINT = (1u<<0) | (1u<<1) | (1u<<2);

    /* Entrar en Power-down */
    PCON = 0x02;   /* PD = 1 */
    __asm { nop }
    __asm { nop }

#ifdef switch_to_PLL
    /* Reenganchar PLL tras el wake-up si se usa */
    switch_to_PLL();
#endif

    /* Limpiar flags de EINT que pudieran haber despertado el dispositivo */
    EXTINT = (1u<<0) | (1u<<1) | (1u<<2);
}
