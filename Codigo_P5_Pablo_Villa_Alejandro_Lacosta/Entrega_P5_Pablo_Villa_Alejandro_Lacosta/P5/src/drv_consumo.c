/* *****************************************************************************
 * P.H.2025: drv_consumo.c
 *
 * Driver de bajo consumo
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Proporciona funciones para que la aplicación gestione modos de espera o sueño.
 *
 ******************************************************************************/ 

#include "drv_consumo.h"
#include "hal_consumo.h"

/* Inicializa el driver de consumo, invoca la inicialización del HAL */
void drv_consumo_iniciar(void) {
    hal_consumo_iniciar();
}

/* Pone el micro en modo espera ligero hasta la siguiente interrupción */
void drv_consumo_esperar(void) {
    hal_consumo_esperar();
}

/* Pone el micro en modo sueño profundo (no retorna) */
void drv_consumo_dormir(void) {
    hal_consumo_dormir();
}
