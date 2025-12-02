/******************************************************************************
 * Fichero: drv_monitor.c
 * Proyecto: P.H.2025
 *
 * Driver/Manejador de MONITORES (depuración)
 *
 * Este módulo permite marcar o desmarcar pines GPIO para su visualización
 * mediante un analizador lógico o medidor de consumo.
 * Similar al driver de LEDs, pero con semántica distinta:
 *   - marcar    = nivel alto (1 lógico)
 *   - desmarcar = nivel bajo (0 lógico)
 *
 * Autores:
 *   Alejandro Lacosta
 *   Pablo Villa
 *
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "drv_monitor.h"
#include "hal_gpio.h"
#include "board.h"

static const uint32_t s_monitor_list[] = MONITOR_LIST;
static const uint32_t s_num_monitores  = MONITOR_NUMBER;

/**
 * @brief Inicializa los monitores: los configura como salidas y los pone a nivel bajo.
 * @return Número de monitores inicializados
 */
uint32_t drv_monitor_iniciar(void) {
    for (uint32_t i = 0; i < s_num_monitores; i++) {
        hal_gpio_sentido(s_monitor_list[i], HAL_GPIO_PIN_DIR_OUTPUT);
        hal_gpio_escribir(s_monitor_list[i], 0); // apagado/inactivo por defecto
    }
    return s_num_monitores;
}

/**
 * @brief Marca (pone a nivel alto lógico) el monitor especificado.
 * @param id Identificador del monitor (1..s_num_monitores)
 */
void drv_monitor_marcar(uint32_t id) {
    if (id == 0 || id > s_num_monitores) return;
    hal_gpio_escribir(s_monitor_list[id - 1], 1);
}

/**
 * @brief Desmarca (pone a nivel bajo lógico) el monitor especificado.
 * @param id Identificador del monitor (1..s_num_monitores)
 */
void drv_monitor_desmarcar(uint32_t id) {
    if (id == 0 || id > s_num_monitores) return;
    hal_gpio_escribir(s_monitor_list[id - 1], 0);
}
