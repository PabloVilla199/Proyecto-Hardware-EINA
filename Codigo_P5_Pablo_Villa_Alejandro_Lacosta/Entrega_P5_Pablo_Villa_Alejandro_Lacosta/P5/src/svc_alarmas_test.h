/******************************************************************************

* Fichero: svc_alarmas_test.h
* Proyecto: P.H. 2025
*
* Descripción:
* Cabecera para los tests del servicio de alarmas. Permite iniciar y verificar
* la funcionalidad de alarmas únicas y periódicas con callbacks asociados.
*
* Autores:
* Alejandro Lacosta
* Pablo Villa
*
* Universidad de Zaragoza
*

*****************************************************************************/

#ifndef SVC_ALARMAS_TEST_H
#define SVC_ALARMAS_TEST_H

/**

* @brief Inicia los tests del servicio de alarmas.
*
* Configura alarmas de prueba (únicas y periódicas), activa LEDs y envía mensajes
* por UART para verificar su correcto funcionamiento.
  */
  void svc_alarmas_test_iniciar(void);

#endif // SVC_ALARMAS_TEST_H
