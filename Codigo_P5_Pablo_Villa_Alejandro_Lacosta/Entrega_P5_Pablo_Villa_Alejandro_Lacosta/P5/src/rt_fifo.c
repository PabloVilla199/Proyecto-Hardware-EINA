/* *****************************************************************************
 * P.H.2025: rt_fifo.c
 *
 * M�dulo de gesti�n de cola FIFO de eventos en tiempo real
 * Autores: Alejandro Lacosta, Pablo Villa
 *
 * Proporciona una interfaz para almacenar, extraer y monitorizar eventos 
 * del sistema en una cola circular con marca temporal.
 *
 ******************************************************************************/


#include "rt_fifo.h"
#include "drv_tiempo.h"
#include "drv_monitor.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include <stdbool.h>
#include "hal_SC.h"



typedef struct {
    EVENTO_T ID_EVENTO;
    uint32_t auxData;
    Tiempo_us_t TS;
} EVENTO;

static EVENTO colaEventos[RT_FIFO_TAM];
static uint8_t indice_insercion = 0;
static uint8_t indice_extraccion = 0;
static uint8_t num_eventos = 0;

static uint32_t monitor_overflow_id = 0;
static uint32_t contador_eventos[EVENT_TYPES] = {0};


static inline uint8_t cola_llena(void) {
    return (num_eventos >= RT_FIFO_TAM);
}

static inline uint8_t cola_vacia(void) {
    return (num_eventos == 0);
}

/* Inicializa la cola FIFO y resetea los contadores de eventos */
void rt_FIFO_inicializar(uint32_t monitor_overflow) {
    indice_insercion = 0;
    indice_extraccion = 0;
    num_eventos = 0;
    monitor_overflow_id = monitor_overflow;

    for (uint8_t i = 0; i < EVENT_TYPES; i++) {
        contador_eventos[i] = 0;
    }
}

/* Encola un nuevo evento junto con su marca temporal interna.
 * Si la cola est� llena, marca el overflow y entra en modo de bajo consumo indefinido. */
void rt_FIFO_encolar(uint32_t ID_evento, uint32_t auxData) {
    if (cola_llena()) {
        drv_monitor_marcar(monitor_overflow_id);
        // Bloqueo en caso de overflow (seg�n guion de pr�cticas)
        while (1) { drv_consumo_dormir(); }
    }

    colaEventos[indice_insercion].ID_EVENTO = (EVENTO_T)ID_evento;
    colaEventos[indice_insercion].auxData = auxData;
    colaEventos[indice_insercion].TS = drv_tiempo_actual_us();

    indice_insercion = (indice_insercion + 1) % RT_FIFO_TAM;
    num_eventos++;
    if (ID_evento < EVENT_TYPES)
        contador_eventos[ID_evento]++;
				
}

/* Extrae el evento m�s antiguo de la cola FIFO.
 * Devuelve 0 si no hay eventos pendientes.
 * En caso contrario, devuelve el n�mero de eventos restantes tras la extracci�n. */
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS) {

	
    if (cola_vacia()) return 0;
	
    *ID_evento = colaEventos[indice_extraccion].ID_EVENTO;
    *auxData   = colaEventos[indice_extraccion].auxData;
    *TS        = colaEventos[indice_extraccion].TS;

    colaEventos[indice_extraccion].ID_EVENTO = ev_VOID;  // Marcar como tratado

    indice_extraccion = (indice_extraccion + 1) % RT_FIFO_TAM;
    num_eventos--;
	
    return num_eventos + 1;
}

/* Devuelve estad�sticas sobre la cola o tipos de evento:
 * - Si ID_evento == ev_VOID ? n�mero de eventos pendientes en la cola.
 * - Si ID_evento < EVENT_TYPES ? n�mero de veces que ha ocurrido ese evento. */
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento) {
    if (ID_evento == ev_VOID)
        return num_eventos;
    else if (ID_evento < EVENT_TYPES)
        return contador_eventos[ID_evento];
    else
        return 0;
}

/* Test interno del m�dulo FIFO.
 * Encola y extrae una secuencia de eventos de prueba verificando el orden y los datos.
 * Devuelve true si todas las operaciones se realizan correctamente. */
bool rt_FIFO_test() {
    EVENTO_T test_ids[5]   = { ev_T_PERIODICO, ev_PULSAR_BOTON, ev_T_PERIODICO, ev_PULSAR_BOTON, ev_T_PERIODICO };
    uint32_t test_aux[5]   = { 10, 20, 30, 40, 50 };
    bool test_ok = true;

    // Inicializar cola
    rt_FIFO_inicializar(0);

    // Encolar eventos
    for (int i = 0; i < 5; i++) {
        rt_FIFO_encolar(test_ids[i], test_aux[i]);
    }

    // Extraer y verificar
    for (int i = 0; i < 5; i++) {
        EVENTO_T id;
        uint32_t aux;
        Tiempo_us_t ts;

        uint8_t restantes = rt_FIFO_extraer(&id, &aux, &ts);

        if (id != test_ids[i] || aux != test_aux[i]) {
            test_ok = false;
        }
    }
    return test_ok;
}
