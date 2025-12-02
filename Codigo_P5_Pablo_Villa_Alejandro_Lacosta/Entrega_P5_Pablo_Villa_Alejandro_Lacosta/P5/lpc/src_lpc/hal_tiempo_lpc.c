/* *****************************************************************************
 * P.H.2025: Temporizadores en LPC2105 – Timer0 (periódico) y Timer1 (tick µs)
 * Cumple la API de hal_tiempo.h usada por drv_tiempo.c, svc_alarmas, etc.
 * - Timer1: free-running a 1 MHz ? hal_tiempo_actual_tick64() devuelve µs (u64)
 * - Timer0: “RTC-like” a 32.768 kHz ? periodos en ticks de 32768 Hz
 * ****************************************************************************/

#include <LPC210x.H>
#include "hal_tiempo.h"

#define PCLK_MHZ   15u
#define PCLK_HZ    (PCLK_MHZ * 1000000u)

/* =================== Tick de alta precisión (Timer1 ? 1 MHz) =================== */

static volatile uint32_t s_overflows_t1 = 0;  // cuenta wraps por MR0
static hal_tiempo_info_t s_info;

/* IRQ de Timer1: se dispara al llegar a MR0 y resetea el contador */
void T1_ISR(void) __irq {
    T1IR = 1u;          // clear MR0
    s_overflows_t1++;
    VICVectAddr = 0;    // ack VIC
}

void hal_tiempo_iniciar_tick(hal_tiempo_info_t *out_info) {
    // Timer1 a 1 MHz: 1 tick = 1 µs
    T1TCR = 2;                                         // reset
    T1PR  = (PCLK_HZ / 1000000u) - 1u;                 // prescaler ? 1 MHz
    T1MR0 = 0xFFFFFFFFu;                               // máximo 32-bit
    T1MCR = (1u<<0) /*MR0I*/ | (1u<<1) /*MR0R*/;       // int + reset
    s_overflows_t1 = 0;

    // VIC para TIMER1 (fuente 5 en LPC2105)
    VICVectAddr1 = (unsigned long)T1_ISR;
    VICVectCntl1 = 0x20 | 5;                           // enable slot + fuente 5
    VICIntEnable |= (1u << 5);

    T1IR  = 0xFF;                                      // limpia flags por si acaso
    T1TCR = 1;                                         // start

    // Contrato simple: hal_tiempo_actual_tick64() devuelve µs ? ticks_per_us = 1
    s_info.ticks_per_us = 1.0f;
    s_info.counter_bits = 32u;
    s_info.counter_max  = 0xFFFFFFFFu;
    if (out_info) *out_info = s_info;
}

/* Lectura atómica 64-bit: detecta wrap entre lecturas de TC */
uint64_t hal_tiempo_actual_tick64(void) {
    uint32_t lo1 = T1TC;
    uint32_t hi1 = s_overflows_t1;
    uint32_t lo2 = T1TC;

    if (lo2 < lo1) {
        // Hubo wrap durante la lectura
        uint32_t hi2 = s_overflows_t1;
        return (((uint64_t)hi2) * (0x100000000ULL)) + lo2;  // µs
    }
    return (((uint64_t)hi1) * (0x100000000ULL)) + lo2;      // µs
}

/* ========================== Reloj periódico (Timer0) ========================== */

static void (*s_cb)(void) = 0;   // callback periódico

void T0_ISR(void) __irq {
    if (s_cb) s_cb();
    T0IR = 1u;         // clear MR0
    VICVectAddr = 0;   // ack VIC
}

/* Configura T0 para que TC incremente a 32.768 kHz y dispare por MR0 */
void hal_tiempo_periodico_config_tick(uint32_t periodo_en_tick) {
    T0TCR = 0;                          // stop / reset config
    if (periodo_en_tick == 0) return;

    // Prescalado: PCLK ? 32.768 kHz (TC incrementa a 32768 Hz)
    T0PR  = (PCLK_HZ / 32768u) - 1u;

    // Match en “ticks RTC”
    T0MR0 = periodo_en_tick - 1u;
    T0MCR = (1u<<0) /*MR0I*/ | (1u<<1) /*MR0R*/;   // int + reset

    // VIC para TIMER0 (fuente 4)
    T0IR = 0xFF;                                     // limpia flags pendientes
    VICVectAddr0 = (unsigned long)T0_ISR;
    VICVectCntl0 = 0x20 | 4;                         // enable slot + fuente 4
}

void hal_tiempo_periodico_set_callback(void (*cb)(void)) {
    s_cb = cb;
}

void hal_tiempo_periodico_enable(bool enable) {
    if (enable) {
        VICIntEnable |= (1u << 4);  // habilita IRQ T0
        T0TCR = 2;                  // reset
        T0TCR = 1;                  // start
    } else {
        T0TCR = 0;                  // stop
        VICIntEnClr = (1u << 4);    // deshabilita IRQ T0
        T0IR = 0xFF;                // limpia flags
    }
}

/* Wrapper con la misma semántica que en nRF */
void hal_tiempo_reloj_periodico_tick(uint32_t periodo_en_tick, void (*cb)(void)) {
    if (periodo_en_tick == 0 || cb == 0) {
        hal_tiempo_periodico_enable(false);
        return;
    }
    hal_tiempo_periodico_set_callback(cb);
    hal_tiempo_periodico_config_tick(periodo_en_tick);
    hal_tiempo_periodico_enable(true);
}
