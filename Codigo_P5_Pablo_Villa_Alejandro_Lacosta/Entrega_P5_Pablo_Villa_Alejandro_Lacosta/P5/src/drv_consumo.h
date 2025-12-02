/* *****************************************************************************
 * P.H.2025: drv_consumo.h
 *
 * Driver de bajo consumo
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Proporciona interfaz para gestionar modos de espera o sueño profundo.
 *
 ******************************************************************************/ 

#ifndef DRV_CONSUMO_H
#define DRV_CONSUMO_H

#include <stdint.h>
#include <stdbool.h>

/* Inicializa el driver de consumo */
void drv_consumo_iniciar(void);

/* Pone el micro en espera ligera (Wait For Interrupt) */
void drv_consumo_esperar(void);

/* Pone el micro en modo sueño profundo (no retorna) */
void drv_consumo_dormir(void);

#endif /* DRV_CONSUMO_H */
