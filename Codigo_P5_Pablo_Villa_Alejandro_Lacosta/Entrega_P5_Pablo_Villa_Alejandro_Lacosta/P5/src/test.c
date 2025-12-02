/******************************************************************************
 * Fichero: test.c
 * Proyecto: Pr�cticas P.H. 2025
 *
 * Descripci�n:
 *   Implementaci�n de la funci�n que ejecuta las distintas sesiones
 *   de pruebas autom�ticas del sistema.
 *
 * Autores:
 * Alejandro Lacosta
 * Pablo Villa 
 * 
 * Universidad de Zaragoza
 *
 *****************************************************************************/

#include "test.h"
#include "test_fifo.h"
#include "drv_consumo.h"
#include "test_blink_v2.h"
#include "test_blink_v3.h"
#include "test_fifo.h"
#include "svc_alarmas_test.h"
#include "drv_botones_test.h"
#include "test_wdt.h"
#include "svc_logs.h"

void ejecutar_sesion_test(uint8_t sesion) {

#if DEBUG
    LOG_INFO("Ejecutando en MODO TESTING");

    switch(sesion) {

        case 1:
            test_blink_v2();
            break;

        case 2:
            test_blink_v3();
            break;

        case 3:
            svc_alarmas_test_iniciar();
            break;

        case 4:
            test_GE();
            break;

        case 5:
            test_wdt_suicidio();
            break;
			  case 6: 
					  test_fifo_run();
					  break;


        default:
            LOG_ERROR("Sesion Test invalida");
            while(1) {
                drv_consumo_esperar();
            }
    }

#else
    LOG_ERROR("ERROR: Debug solicitado pero TESTING no definido");
    while(1) {
        drv_consumo_esperar();
    }
#endif
}
