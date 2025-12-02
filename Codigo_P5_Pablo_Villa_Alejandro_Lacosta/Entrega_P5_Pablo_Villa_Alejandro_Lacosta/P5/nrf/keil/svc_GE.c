/* *****************************************************************************
 * P.H.2025: svc_GE.c
 * Servicio Gestor de Eventos (SVC_GE)
 *
 * Gestiona la tabla de suscripciones de eventos del sistema.
 * Cada evento puede tener varios callbacks asociados, ordenados por prioridad.
 *
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Funciones principales:
 *  - svc_GE_suscribir(): Registra un callback a un evento.
 *  - svc_GE_cancelar(): Elimina un callback de un evento y compacta la lista.
 * 
 *
 * Notas:
 *  - Si se intenta suscribir más de rt_GE_MAX_SUSCRITOS callbacks a un evento,
 *    el sistema entra en bucle infinito (overflow).
 *  - La prioridad más baja (0) se ejecuta primero.
 * *****************************************************************************/

#include "svc_GE.h"
#include "drv_leds.h"

// -----------------------------------------------------------------------------
// Suscribe una función callback a un evento con prioridad dada
// Si la tabla está llena, entra en bucle infinito (overflow)
// -----------------------------------------------------------------------------
#include "svc_GE.h"
#include "drv_leds.h"

Suscripcion_t s_tabla[rt_GE_MAX_SUSCRITOS];

void svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad,
                      SVC_CALLBACK_T funcion_callback) {
    // Contar cuántas suscripciones están activas
    uint8_t cont = 0;
												
    for (int i = 0; i < rt_GE_MAX_SUSCRITOS; i++) {
        if (s_tabla[i].activa)
            cont++;
    }

    // Si está llena ? overflow crítico
    if (cont >= rt_GE_MAX_SUSCRITOS) {
        drv_led_establecer(1, LED_ON);
        while (1) { } // bucle infinito
    }

    // Buscar una posición libre y suscribir
    for (int i = 0; i < rt_GE_MAX_SUSCRITOS; i++) {
        if (!s_tabla[i].activa) {
            s_tabla[i].activa = true;
					  s_tabla[i].evento = ID_evento;
            s_tabla[i].f_callback = funcion_callback;
            s_tabla[i].prioridad = prioridad;
            return;
        }
    }
}

// -----------------------------------------------------------------------------
// Cancela una suscripción y compacta la lista del evento
// -----------------------------------------------------------------------------
void svc_GE_cancelar(EVENTO_T ID_evento,
                     SVC_CALLBACK_T funcion_callback) {

    for (int i = 0; i < rt_GE_MAX_SUSCRITOS; i++) {
        if (s_tabla[i].activa &&
            s_tabla[i].evento == ID_evento &&
            s_tabla[i].f_callback == funcion_callback) {

            s_tabla[i].activa = false;

            // Compactar la tabla (mover hacia arriba las siguientes activas)
            for (int j = i; j < rt_GE_MAX_SUSCRITOS - 1; j++) {
                s_tabla[j] = s_tabla[j + 1];
            }

            s_tabla[rt_GE_MAX_SUSCRITOS - 1].activa = false;
            s_tabla[rt_GE_MAX_SUSCRITOS - 1].evento = 0;
            s_tabla[rt_GE_MAX_SUSCRITOS - 1].f_callback = NULL;
            s_tabla[rt_GE_MAX_SUSCRITOS - 1].prioridad = 0;

            return; // Solo se elimina la primera coincidencia
        }
    }
}