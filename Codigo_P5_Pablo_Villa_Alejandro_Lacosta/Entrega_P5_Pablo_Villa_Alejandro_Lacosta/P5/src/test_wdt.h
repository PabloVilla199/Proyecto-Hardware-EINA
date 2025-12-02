/******************************************************************************
 * Fichero: test_wdt.h
 * Proyecto: Prácticas P.H. 2025
 *
 * Descripción:
 *   Declaración de la función de prueba del Watchdog Timer. Esta prueba ejecuta
 *   un ciclo donde el WDT se alimenta durante unos segundos y posteriormente se
 *   deja de alimentar para verificar que el sistema se reinicia automáticamente.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa 
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#ifndef TEST_WDT_H
#define TEST_WDT_H

/**
 * @brief Prueba de concepto del Watchdog.
 * 1. Inicia el WDT a 2 segundos.
 * 2. Lo alimenta durante 10 segundos (LED parpadeando).
 * 3. Deja de alimentarlo (Simula bloqueo).
 * 4. El sistema debería reiniciarse solo.
 */
void test_wdt_suicidio(void);

#endif // TEST_WDT_H
