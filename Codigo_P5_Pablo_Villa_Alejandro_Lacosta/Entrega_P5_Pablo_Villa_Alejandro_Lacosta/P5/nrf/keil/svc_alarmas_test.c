#include "svc_alarmas_test.h"
#include "svc_alarmas.h"
#include "drv_tiempo.h"
#include "drv_leds.h"
#include "drv_uart.h"
#include "drv_consumo.h"

#define LED1 1
#define LED2 2
#define LED3 3

static volatile int contador_periodico = 0;

// Callback que se llama al dispararse una alarma
static void test_callback(EVENTO_T ev, uint32_t auxData) {
    // Aquí usamos directamente auxData para saber qué LED tocar
    // Periodicidad: LED2 es periódica, LED1 y LED3 no
    if (auxData == LED2) {
        drv_led_conmutar((LED_id_t)auxData); // parpadea
    } else {
        drv_led_establecer((LED_id_t)auxData, LED_ON); // se queda encendido
    }

    contador_periodico++;
}

// Test de alarmas
void svc_alarmas_test_iniciar(void) {
    drv_tiempo_iniciar();
    drv_leds_iniciar();
    drv_uart_init();
	
		uint32_t cod = svc_alarma_codificar(false, 1500, 0);
		uint32_t retardo = (cod >> 8) & 0x00FFFFFF;
		char buffer[64];
		sprintf(buffer, "Codificado=%lu, decodificado=%lu\n", cod, retardo);
		drv_uart_send(buffer);



    // Inicializar el servicio de alarmas
    svc_alarma_iniciar(0, test_callback, ev_T_PERIODICO);

    // Activar tres alarmas al mismo tiempo (1,5 s)
    uint32_t cod_unica1 = svc_alarma_codificar(false, 1500, 0); // LED1
    uint32_t cod_periodica = svc_alarma_codificar(true, 500, 0); // LED2
    uint32_t cod_unica2 = svc_alarma_codificar(false, 1500, 0); // LED3

    svc_alarma_activar(cod_unica1, ev_T_PERIODICO, LED1);
    svc_alarma_activar(cod_periodica, ev_T_PERIODICO, LED2);
    svc_alarma_activar(cod_unica2, ev_T_PERIODICO, LED3);

    // Bucle principal de test
    for (int i = 0; i < 50; i++) { // aprox 12,5 s
        drv_consumo_esperar();
    }
}
