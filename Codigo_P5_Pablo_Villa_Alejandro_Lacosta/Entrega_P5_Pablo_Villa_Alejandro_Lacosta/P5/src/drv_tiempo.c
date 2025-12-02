/* *****************************************************************************
 * P.H.2025: Driver/Manejador de los temporizadores
 * suministra los servicios independientemente del hardware
 *
 * usa los servicos de hal_tiempo.h: 
 * Autores: Alejandro Lacosta, Pablo Villa 
 */
 
#include "drv_tiempo.h"
#include "hal_tiempo.h"

static hal_tiempo_info_t info;
static bool iniciado = false;
static void (*s_callback_app)(void);
static uint32_t s_ID_evento = 0;

/**
 * Inicializa el reloj y empieza a contar
 */
bool drv_tiempo_iniciar(void) {
    hal_tiempo_iniciar_tick(&info);
    iniciado = true;
    return iniciado; 
}

/**
 * Tiempo desde que se inició el temporizador en microsegundos
 */
Tiempo_us_t drv_tiempo_actual_us(void) {
    if (!iniciado) return 0;

    uint64_t ticks = hal_tiempo_actual_tick64();
    return ticks / info.ticks_per_us;  // conversión de ticks -> microsegundos
}

/**
 * Tiempo desde que se inició el temporizador en milisegundos
 */
Tiempo_ms_t drv_tiempo_actual_ms(void) {
    if (!iniciado) return 0;
    return (Tiempo_ms_t)(hal_tiempo_actual_tick64() / 1000ULL);
}

/**
 * Esperar un cierto tiempo en milisegundos (bloqueante)
 */
void drv_tiempo_esperar_ms(Tiempo_ms_t ms) {
    if (!iniciado) return;

    uint64_t inicio = drv_tiempo_actual_ms();

    while ((drv_tiempo_actual_ms() - inicio) < ms) {
        // bucle activo (busy wait)
    }
}

/**
 * Esperar hasta un determinado tiempo (en ms), devuelve el tiempo actual
 */
Tiempo_ms_t drv_tiempo_esperar_hasta_ms(Tiempo_ms_t deadline_ms) {
    if (!iniciado) return 0;

    while (drv_tiempo_actual_ms() < deadline_ms) {
        // espera bloqueante hasta que el tiempo actual >= deadline
    }
    return drv_tiempo_actual_ms();
}

void drv_tiempo_periodico_ms(Tiempo_ms_t ms,void(*funcion_callback_app)(), uint32_t ID_evento){
	if (ms == 0) return;
	s_callback_app = funcion_callback_app;
  s_ID_evento = ID_evento;

  uint32_t periodo_en_ticks = (uint32_t)((ms * 32768u) / 1000u);
  hal_tiempo_reloj_periodico_tick(periodo_en_ticks, s_callback_app);

}

uint32_t drv_tiempo_get_evento_id(void) {
    return s_ID_evento;
}
