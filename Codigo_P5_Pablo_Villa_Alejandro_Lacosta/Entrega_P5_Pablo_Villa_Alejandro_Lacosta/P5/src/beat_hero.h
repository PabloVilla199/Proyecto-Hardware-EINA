#ifndef BEAT_HERO_H
#define BEAT_HERO_H

#include <stdint.h>
#include <stdbool.h>


// Estados del juego
typedef enum {
    e_INIT,
    e_SHOW_SEQUENCE,
    e_WAIT_FOR_INPUT,
    e_FIN_PARTIDA,
		e_ESPERANDO_REINICIO
} estado_juego_t;


// Inicializar el juego
void beat_hero_iniciar(void);

// juego + Estadisticas 
void beat_hero_extend_iniciar(void);

#endif
