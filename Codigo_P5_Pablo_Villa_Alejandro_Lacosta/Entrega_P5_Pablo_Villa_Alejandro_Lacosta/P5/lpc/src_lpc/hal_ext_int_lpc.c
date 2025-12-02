/* *****************************************************************************
 * P.H.2025: hal_ext_int_lpc.c
 * HAL de interrupciones externas para LPC2105 (EINT0..EINT2)
 *
 * Pines mapeados:
 *  - P0.16 -> EINT0 (VIC canal 14)
 *  - P0.14 -> EINT1 (VIC canal 15)
 *  - P0.15 -> EINT2 (VIC canal 16)
 * *****************************************************************************/

#include <LPC210x.H>
#include <stdint.h>
#include "hal_ext_int.h"

#ifndef EXTMODE
#  define EXTMODE   (*((volatile unsigned long *)0xE01FC148))
#endif
#ifndef EXTPOLAR
#  define EXTPOLAR  (*((volatile unsigned long *)0xE01FC14C))
#endif

/* Pines y canales VIC */
#define PIN_EINT0   16u
#define PIN_EINT1   14u
#define PIN_EINT2   15u

#define VIC_CH_EINT0 14u
#define VIC_CH_EINT1 15u
#define VIC_CH_EINT2 16u

/* Callback de usuario */
static hal_ext_int_callback_t s_cb = 0;

/* Helpers internos */
static inline void eint_clear_flag(unsigned bit) { EXTINT = (1u << bit); }
static inline void vic_enable(unsigned chan)     { VICIntEnable |=  (1u << chan); }
static inline void vic_disable(unsigned chan)    { VICIntEnClr  =   (1u << chan); }

/* ============================ ISRs ============================ */

/**
 * @brief ISR de EINT0 (P0.16)
 */
void EINT0_ISR(void) __irq {
    eint_clear_flag(0);
    vic_disable(VIC_CH_EINT0);
    if (s_cb) s_cb(HAL_EXT_INT_0);
    VICVectAddr = 0;
}

/**
 * @brief ISR de EINT1 (P0.14)
 */
void EINT1_ISR(void) __irq {
    eint_clear_flag(1);
    vic_disable(VIC_CH_EINT1);
    if (s_cb) s_cb(HAL_EXT_INT_1);
    VICVectAddr = 0;
}

/**
 * @brief ISR de EINT2 (P0.15)
 */
void EINT2_ISR(void) __irq {
    eint_clear_flag(2);
    vic_disable(VIC_CH_EINT2);
    if (s_cb) s_cb(HAL_EXT_INT_2);
    VICVectAddr = 0;
}

/* ============================ API ============================ */

/**
 * @brief Inicializa la HAL de interrupciones externas.
 *
 * Configura pines, edge/falling, limpia flags y vectoriza ISR en VIC.
 * @param cb Callback que se ejecuta al recibir la interrupción.
 */
void hal_ext_int_iniciar(hal_ext_int_callback_t cb){
    s_cb = cb;

    /* Configurar función EINT en pines correspondientes */
    PINSEL1 &= ~(3u << 0);  PINSEL1 |=  (1u << 0);  /* P0.16 -> EINT0 */
    PINSEL0 &= ~(3u << 28); PINSEL0 |=  (2u << 28); /* P0.14 -> EINT1 */
    PINSEL0 &= ~(3u << 30); PINSEL0 |=  (2u << 30); /* P0.15 -> EINT2 */

    /* Pines como entrada */
    IODIR &= ~((1u << PIN_EINT0) | (1u << PIN_EINT1) | (1u << PIN_EINT2));

    /* Edge falling (HW real) */
    EXTMODE  |=  (1u<<0) | (1u<<1) | (1u<<2);
    EXTPOLAR &= ~((1u<<0) | (1u<<1) | (1u<<2));

    /* Limpiar flags antes de vectorizar */
    EXTINT = (1u<<0) | (1u<<1) | (1u<<2);

    /* Vectorizar en VIC (ejemplo: slots 6,7,8) */
    VICVectAddr6 = (unsigned long)EINT0_ISR;  VICVectCntl6 = 0x20 | VIC_CH_EINT0;
    VICVectAddr7 = (unsigned long)EINT1_ISR;  VICVectCntl7 = 0x20 | VIC_CH_EINT1;
    VICVectAddr8 = (unsigned long)EINT2_ISR;  VICVectCntl8 = 0x20 | VIC_CH_EINT2;
}

/**
 * @brief Habilita la interrupción externa indicada.
 */
void hal_ext_int_habilitar(hal_ext_int_id_t id){
    switch (id) {
        case HAL_EXT_INT_0:  eint_clear_flag(0); vic_enable(VIC_CH_EINT0); break;
        case HAL_EXT_INT_1:  eint_clear_flag(1); vic_enable(VIC_CH_EINT1); break;
        case HAL_EXT_INT_2:  eint_clear_flag(2); vic_enable(VIC_CH_EINT2); break;
        default: break;
    }
}

/**
 * @brief Deshabilita la interrupción externa indicada.
 */
void hal_ext_int_deshabilitar(hal_ext_int_id_t id){
    switch (id) {
        case HAL_EXT_INT_0:  vic_disable(VIC_CH_EINT0); break;
        case HAL_EXT_INT_1:  vic_disable(VIC_CH_EINT1); break;
        case HAL_EXT_INT_2:  vic_disable(VIC_CH_EINT2); break;
        default: break;
    }
}

/**
 * @brief Lee el valor lógico del pin P0.n indicado.
 * @param p0_bit Número de bit en el puerto 0
 * @return 0 o 1
 */
uint32_t hal_ext_int_leer_pin(uint8_t p0_bit){
    return (IOPIN >> p0_bit) & 1u;
}

/**
 * @brief Permite que la interrupción despierte al micro desde Power-down.
 */
void hal_ext_int_habilitar_despertar(hal_ext_int_id_t id){
    if (id == HAL_EXT_INT_0)      EXTWAKE |= (1u<<0);
    else if (id == HAL_EXT_INT_1) EXTWAKE |= (1u<<1);
    else if (id == HAL_EXT_INT_2) EXTWAKE |= (1u<<2);
}

/**
 * @brief Deshabilita la función de wake-up desde Power-down para la interrupción.
 */
void hal_ext_int_deshabilitar_despertar(hal_ext_int_id_t id){
    if (id == HAL_EXT_INT_0)      EXTWAKE &= ~(1u<<0);
    else if (id == HAL_EXT_INT_1) EXTWAKE &= ~(1u<<1);
    else if (id == HAL_EXT_INT_2) EXTWAKE &= ~(1u<<2);
}
