/******************************************************************************
* Fichero: drv_botones_test.h
* Proyecto: P.H. 2025
*
* Descripción:
* módulo de prueba del driver de botones. Permite ejecutar
* tests completos del driver, incluyendo control de rebotes, FSM, integración
* con servicios de alarma, logs y bajo consumo.
*
* Autores:
* Alejandro Lacosta
* Pablo Villa
*
* Universidad de Zaragoza
*

*****************************************************************************/

#ifndef DRV_BOTONES_TEST_H
#define DRV_BOTONES_TEST_H

/**

* @brief Ejecuta el test completo del driver de botones.
*
* Inicializa todos los servicios necesarios (tiempo, alarmas, consumo, logs)
* y entra en un bucle principal que monitoriza los estados de los botones.
*
*/
  int test_GE(void);

#endif /* DRV_BOTONES_TEST_H */
