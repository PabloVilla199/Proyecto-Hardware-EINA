/* *****************************************************************************
 * P.H.2025: hal_tiempo_nrf.c
 * HAL de temporizador para nRF52840 DK
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Implementa:
 *  - Reloj monótono de 24-bit (SysTick)
 *  - Reloj periódico con callback (RTC1)
 *
 * Notas:
 *  - SysTick: alta precisión (usado para medir tiempo en µs o ms)
 *  - RTC1: bajo consumo, usado para temporización periódica (32768 Hz)
 *  - Prescaler = 0 ? frecuencia base del RTC = 32768 Hz
 *
 ******************************************************************************/

	#include "hal_tiempo.h"
	#include "nrf.h"

	#define TICKS_PER_US   16u
	#define COUNTER_BITS   32u
	#define COUNTER_MAX    0xFFFFFFFFu
	#define MAX_SYSTICK_RELOAD 0x00FFFFFFu

//	static hal_tiempo_info_t s_info;               // información de alta precisión (SysTick)
	static hal_tiempo_info_t s_info_per;     // información de baja frecuencia (RTC1)
	static void (*s_periodic_callback)(void);      // callback periódico
	static volatile uint64_t s_tick64 = 0;      // contador de milisegundos (64 bits)
	static volatile uint32_t s_ticks_us = 0;    // ticks por microsegundo
	
	// SysTick 

	// ============================================================================
	// SysTick Handler se ejecuta automáticamente cada 1 ms
	// ============================================================================
	void SysTick_Handler(void) {
    s_tick64++;  // incrementa cada milisegundo
	}
	
	/* ============================================================================
 * hal_tiempo_iniciar_tick
 * ============================================================================
 * @brief Inicializa el temporizador SysTick como reloj monótono de alta precisión.
 * 
 * Configura el SysTick para generar interrupciones cada 1 ms, usando el reloj
 * principal de la CPU (SystemCoreClock). Este temporizador se usa para medir
 * intervalos de tiempo precisos y obtener marcas temporales absolutas.  
 *
 * @param [in/out] Puntero a una estructura `hal_tiempo_info_t` donde se
 *                      almacenará la información del reloj (ticks por µs, etc).
 * 
 * @return Ninguno.
 * 
 * @details
 *  - Inicia el oscilador de alta frecuencia (HFCLK).
 *  - Configura el valor de recarga para generar interrupciones cada 1 ms.
 *  - Guarda la frecuencia de ticks por microsegundo en `info->ticks_per_us`.
 * ============================================================================
 */

	void hal_tiempo_iniciar_tick(hal_tiempo_info_t *info) {
		
		NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
        // Esperar al encendido (~1–2 ms)
    }
		
    uint32_t core_clock = SystemCoreClock;            // frecuencia CPU (Hz)
    uint32_t reload_val = (core_clock / 1000u) - 1u;  // 1 ms (64e6 / 1000) - 1 = 63999

    if (reload_val > 0x00FFFFFFu)                     // límite de 24 bits
        reload_val = 0x00FFFFFFu;

    info->ticks_per_us  = core_clock / 1000000u;      // ticks/µs 
    s_tick64 = 0;

    // --- Configurar registros del SysTick ---
    SysTick->LOAD = reload_val;   // Valor de recarga cada 1 ms
    SysTick->VAL  = 0;            // Reset del contador actual
    SysTick->CTRL = (1u << 2) |   // CLKSOURCE = CPU clock
                    (1u << 1) |   // TICKINT = interrupciones activadas (opcional)
                    (1u << 0);    // ENABLE = contador activo
	}

/* ============================================================================
 * hal_tiempo_actual_tick64
 * ============================================================================
 * @brief Devuelve el tiempo actual transcurrido desde el arranque (en µs).
 * 
 * @return Valor de tiempo actual (en microsegundos) desde el inicio del sistema.
 * 
 * @details
 *  - Combina el contador de milisegundos (`s_tick64`) con la fracción restante
 *    del SysTick (`SysTick->VAL`).
 *  - Ofrece resolución sub-milisegundo calculando la fracción de tick actual.
 *  - Ideal para mediciones de tiempo precisas (del orden de µs).
 * ============================================================================
 */
	uint64_t hal_tiempo_actual_tick64(void) {
    uint32_t val  = SysTick->VAL;
    uint32_t load = SysTick->LOAD;

    // Tiempo base (en µs): cada tick equivale a 1 ms = 1000 µs
    uint64_t base_us = s_tick64 * 1000ULL;

    // Fracción sub-ms convertida a µs
    uint32_t partial_us = ((load - val) * 1000000ULL) / SystemCoreClock;

    return base_us + partial_us;
	}

	/* --- Reloj periódico con callback usando RTC1 --- */
	
/* ============================================================================
 * hal_tiempo_iniciar_tick_periodico
 * ============================================================================
 * @brief Inicializa el reloj de baja frecuencia (RTC1) como base temporal lenta.
 * 
 * Arranca el oscilador de baja frecuencia (LFCLK) basado en cristal externo
 * de 32.768 kHz y configura el RTC1 como fuente de tiempo periódica eficiente.
 * 
 * @param[out] out_info_per  Puntero a estructura `hal_tiempo_info_t` que se 
 *                           llena con la información del reloj (ticks/µs).
 * 
 * @return Ninguno.
 * 
 * @details
 *  - Este reloj es más lento pero de muy bajo consumo.
 *  - Se usa como base para temporizadores periódicos (no para medición exacta).
 * ============================================================================
 */
	
	void hal_tiempo_iniciar_tick_periodico(hal_tiempo_info_t *out_info_per) {
    // Arrancar reloj de baja frecuencia (LFCLK)
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal;   // usar cristal externo
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);

    // Configurar RTC1 como base de tiempo lenta
    NRF_RTC1->TASKS_STOP  = 1;
    NRF_RTC1->TASKS_CLEAR = 1;
    NRF_RTC1->PRESCALER   = 0;                        // 32.768 kHz
    out_info_per->ticks_per_us = 0.032768f;           // 1 tick = 30.517 µs aprox
}

	void hal_tiempo_reloj_periodico_tick(uint32_t periodo_en_tick,void(*funcion_callback_drv)()){
		
		 if (periodo_en_tick == 0) {
					hal_tiempo_periodico_enable(false);  // detener el timer si periodo 0
					return;
			}
		 
			hal_tiempo_iniciar_tick_periodico(&s_info_per);
			hal_tiempo_periodico_config_tick(periodo_en_tick);
			hal_tiempo_periodico_set_callback(funcion_callback_drv);
			hal_tiempo_periodico_enable(true);
	}


	/**
	 * @brief Handler de interrupción de TIMER0
	 *
	 * Llama al callback registrado cada vez que el contador alcanza CC[0].
	 */
	void RTC1_IRQHandler(void) {
    if (NRF_RTC1->EVENTS_COMPARE[0]) {
        NRF_RTC1->EVENTS_COMPARE[0] = 0;
        NRF_RTC1->TASKS_CLEAR = 1;       
        if (s_periodic_callback) s_periodic_callback();
    }
}

	/**
	 * @brief Configura el periodo del reloj periódico en ticks
	 *
	 * @param periodo_en_tick Número de ticks para el periodo
	 */
	void hal_tiempo_periodico_config_tick(uint32_t periodo_en_tick) {
    NRF_RTC1->TASKS_STOP = 1;
    NRF_RTC1->TASKS_CLEAR = 1;

    NRF_RTC1->PRESCALER = 0;              // prescaler = 0 -->  32.768 kHz
    NRF_RTC1->CC[0] = periodo_en_tick;    // valor de comparación
    NRF_RTC1->EVENTS_COMPARE[0] = 0;
    NRF_RTC1->EVTENSET = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Msk;

	}

	/**
	 * @brief Registra la función callback a ejecutar en cada interrupción periódica
	 *
	 * @param cb Puntero a la función callback
	 */
	void hal_tiempo_periodico_set_callback(void (*cb)()) {
    s_periodic_callback = cb;
	}

	/**
	 * @brief Habilita o deshabilita el reloj periódico
	 *
	 * @param enable true: arranca el temporizador / false: lo detiene
	 */
	void hal_tiempo_periodico_enable(bool enable) {
    if (enable) {
        NVIC_EnableIRQ(RTC1_IRQn);
        NRF_RTC1->TASKS_START = 1;
    } else {
        NRF_RTC1->TASKS_STOP = 1;
        NVIC_DisableIRQ(RTC1_IRQn);
    }
	}

