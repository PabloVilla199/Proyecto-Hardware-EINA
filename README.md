**Proyecto**: Beat Hero - Asignatura de Hardware

Este repositorio contiene el proyecto realizado en la asignatura de hardware cuyo objetivo es implementar el juego "BeatHero" (también referido como Beat Hero). El proyecto está pensado como ejercicio práctico de arquitectura por capas y portabilidad de drivers sobre dos plataformas objetivo: `LPC2105` y plataformas `nRF52` (p. ej. nRF52840 / nRF520840 según la placa disponible).

**Propósito**: demostrar diseño de software embebido modular y reusable mediante separación en capas (HAL, drivers, servicios y aplicación). El código incluye la implementación del juego, los drivers, HALs para cada familia de microcontroladores y utilidades/servicios para la ejecución del juego.

**Archivos clave y recursos**
- **Código fuente**: `P5_Pablo_Villa_Alejandro_Lacosta/P5/src/` contiene la implementación principal (juego, drivers y servicios).
- **Proyectos Keil**:
  - `P5_Pablo_Villa_Alejandro_Lacosta/P5/lpc/keil/` — proyecto para `LPC2105` (`.uvproj`, `.uvopt`).
  - `P5_Pablo_Villa_Alejandro_Lacosta/P5/nrf/keil/` — proyecto para `nRF52` (`.uvprojx`, `.uvoptx`).
- **Manuales e infografías**:
  - `ManualUsuario.pdf` — instrucciones de ejecución y uso (hay una copia en la raíz y otra en la carpeta del P5).
  - `Infografia_Beat_Hero.pdf` — resultados y rendimientos.
- **Presentación**: `Bit_Beat Hero_ Demo y Arquitectura.pptx` — presentación del proyecto.
- **Diagramas de estados** (explican el funcionamiento de los programas de estado/millis del proyecto):
  - `Diagrama_Estados_Botones.png`
  - `Diagrama_Estados_BitCounter.png`
  - `Diagrama_Estados_Beat_hero.png`

**Estructura general (resumen)**
- `src/`: implementación portable y módulos del juego.
- `lpc/src_lpc/`, `nrf/src_nrf/`: adaptaciones específicas de hardware (HAL y archivos de arranque para cada plataforma).
- `lpc/keil/`, `nrf/keil/`: proyectos de Keil uVision para compilar/depurar en cada plataforma.

## Arquitectura por capas

El proyecto sigue una **arquitectura en capas estricta** para facilitar la reutilización y la independencia del hardware. La comunicación entre capas es **unidireccional descendente**: cada capa solo puede invocar servicios de la capa inmediatamente inferior, nunca de capas superiores.

```
┌─────────────────────────────────────────────────────────────┐
│            CAPA 4: APLICACIÓN / JUEGO                       │
│  beat_hero.c, beat_hero_extend.c, bit_counter_strike.c     │
│  main.c                                                      │
│  • Lógica del juego (secuencias, puntuación, estados)      │
│  • Punto de entrada del programa                            │
└──────────────────────┬──────────────────────────────────────┘
                       │ llama a ↓
┌─────────────────────────────────────────────────────────────┐
│       CAPA 3: SERVICIOS / RUNTIME (svc_*, rt_*)            │
│  • svc_GE.c/.h      - Gestor de eventos (suscripciones)    │
│  • rt_GE.c/.h       - Runtime/despachador de eventos       │
│  • rt_fifo.c/.h     - Cola FIFO de eventos con timestamp   │
│  • svc_alarmas.c/.h - Temporizadores y alarmas periódicas  │
│  • svc_logs.c/.h    - Sistema de logging                   │
│  • Coordinan flujos de eventos entre módulos               │
└──────────────────────┬──────────────────────────────────────┘
                       │ usa ↓
┌─────────────────────────────────────────────────────────────┐
│           CAPA 2: DRIVERS (drv_*)                          │
│  • drv_botones.c/.h   - FSM de botones con antirrebote     │
│  • drv_leds.c/.h      - Control de LEDs (on/off/toggle)    │
│  • drv_tiempo.c/.h    - Tiempo absoluto (us/ms)            │
│  • drv_uart.c/.h      - Comunicación serie (debug)         │
│  • drv_wdt.c          - Watchdog timer                      │
│  • drv_consumo.c/.h   - Medición de consumo energético     │
│  • drv_monitor.c/.h   - Monitorización de señales GPIO     │
│  • APIs estables e independientes del hardware             │
└──────────────────────┬──────────────────────────────────────┘
                       │ invoca ↓
┌─────────────────────────────────────────────────────────────┐
│     CAPA 1: HAL (Hardware Abstraction Layer)              │
│  • hal_gpio.h/.c     - Acceso a GPIO (leer/escribir/dir)   │
│  • hal_tiempo.h/.c   - Timers hardware (us/ms)             │
│  • hal_uart.h/.c     - UART física                         │
│  • hal_wdt.h/.c      - Watchdog hardware                   │
│  • hal_ext_int.h/.c  - Interrupciones externas             │
│  • hal_random.h/.c   - Generador de números aleatorios     │
│  • hal_consumo.h/.c  - Medición HW de consumo              │
│  • hal_SC.h/.c       - Secciones críticas (disable IRQ)    │
│  • Implementaciones específicas por plataforma             │
└──────────────────────┬──────────────────────────────────────┘
                       │ accede a ↓
┌─────────────────────────────────────────────────────────────┐
│         CAPA 0: HARDWARE / REGISTROS                       │
│  • LPC2105:  registros IOSET, IOCLR, PINSEL, etc.         │
│  • nRF52840: periféricos NRF_P0, NRF_TIMER, etc.          │
│  • board.h / board_lpc.h / board_nrf52840dk.h              │
│  • Definiciones de pines, configuraciones de placa        │
└─────────────────────────────────────────────────────────────┘
```

### Detalles de cada capa:

#### **CAPA 1: HAL (Hardware Abstraction Layer)**
`hal_*.c/h` contienen las abstracciones de más bajo nivel (GPIO, UART, WDT, temporizadores). Cada plataforma (LPC, nRF) implementa su versión del HAL:
- **LPC2105**: `lpc/src_lpc/hal_*_lpc.c` (ej: `hal_gpio_lpc.c`, `hal_tiempo_lpc2105.c`)
- **nRF52**: `nrf/src_nrf/hal_*_nrf.c` (ej: `hal_gpio_nrf.c`)

La HAL es la **única capa** que interactúa directamente con los registros/periféricos del MCU. Proporciona una API común (`hal_gpio.h`) implementada de forma diferente en cada plataforma.

**Ejemplo**: `hal_gpio_escribir(pin, valor)` en LPC2105 escribe en `IOSET/IOCLR`, mientras que en nRF52 escribe en `NRF_P0->OUTSET/OUTCLR`.

#### **CAPA 2: DRIVERS (drv_*)**
Sobre la HAL se sitúan los drivers que exponen APIs funcionales de más alto nivel:
- `drv_botones.c/.h`: gestión de entrada por botones, implementa FSM con estados (E_ESPERANDO, E_REBOTES, E_MUESTREO, E_SALIDA), debounce y mapeo a eventos del juego. Usa `hal_gpio` y `hal_ext_int`.
- `drv_leds.c/.h`: abstrae la manipulación de LEDs del tablero/juego. Maneja lógica activa-alta/activa-baja automáticamente.
- `drv_tiempo.c/.h`: funciones de temporización (`drv_tiempo_actual_ms()`, `drv_tiempo_esperar_ms()`) usadas por el motor de estados. Usa `hal_tiempo`.
- `drv_uart.c/.h`: comunicación serie para debug/telemetría. Envoltorio sobre `hal_uart`.
- `drv_wdt.c` / `drv_wtd.c`: watchdog y control de reinicio. Usa `hal_wdt`.
- `drv_consumo.c/.h`: medición de consumo energético. Usa `hal_consumo`.

**Característica clave**: Los drivers son **independientes del hardware** — su API pública no cambia entre plataformas.

#### **CAPA 3: SERVICIOS / MIDDLEWARES (svc_*, rt_*)**
Componentes reutilizables que implementan lógica de coordinación y soporte:
- `rt_fifo.c/.h`: cola FIFO de eventos con timestamp para comunicación asíncrona entre ISRs y tareas principales.
- `rt_GE.c/.h`: **Runtime del Gestor de Eventos** — despachador central que extrae eventos de la FIFO y los distribuye a los suscriptores.
- `svc_GE.c/.h`: **Servicio de Gestor de Eventos** — tabla de suscripciones que permite registrar callbacks por evento con prioridades (0 = más alta).
- `svc_alarmas.c/.h`: gestión de alarmas y temporizadores de alto nivel. Genera eventos periódicos (`ev_T_PERIODICO`).
- `svc_logs.c/.h`: registro de eventos y ayuda al debug. Sistema de logging estructurado.

**Patrón de diseño**: arquitectura basada en eventos (event-driven). Los módulos se comunican mediante eventos encolados en `rt_fifo`, el `rt_GE` despacha eventos a los callbacks registrados en `svc_GE`.

#### **CAPA 4: APLICACIÓN / JUEGO**
`beat_hero.c`, `beat_hero_extend.c`, `bit_counter_strike.c`, `main.c` contienen la lógica del juego:
- `main.c`: punto de entrada, inicializa drivers/servicios y arranca el juego.
- `beat_hero.c`: implementa la FSM del juego (estados: e_INIT, e_SHOW_SEQUENCE, e_WAIT_FOR_INPUT, e_FIN_PARTIDA, e_ESPERANDO_REINICIO).
- `beat_hero_extend.c`: versión extendida con estadísticas.
- `bit_counter_strike.c`: variante del juego.

Usa los servicios (`svc_GE`, `svc_alarmas`) y drivers (`drv_botones`, `drv_leds`, `drv_tiempo`) para interactuar con el hardware.

### Flujo de comunicación entre capas:

```
main.c (App)
    ↓ inicializa
drv_tiempo_iniciar() → hal_tiempo_iniciar() → configura TIMER0 (LPC) / NRF_TIMER (nRF)
    ↓
rt_GE_iniciar() → svc_alarma_iniciar() → drv_tiempo_periodico_ms()
    ↓
[ISR Timer] → rt_FIFO_encolar(ev_T_PERIODICO)
    ↓
rt_GE_lanzador() → extrae de rt_fifo → busca callbacks en svc_GE → ejecuta callback
    ↓
callback_boton() (App) → drv_leds_establecer() → hal_gpio_escribir() → IOSET/IOCLR
```

### Ventajas de esta arquitectura:

1. **Portabilidad**: para portar a otra placa solo se implementa la capa HAL. Los drivers, servicios y aplicación no cambian.
2. **Reusabilidad**: drivers y servicios se pueden reutilizar en otros proyectos embebidos.
3. **Testabilidad**: cada capa se puede probar independientemente con mocks de la capa inferior.
4. **Mantenibilidad**: cambios en el hardware solo afectan a la HAL, no propagan cambios a capas superiores.
5. **Separación de responsabilidades**: cada módulo tiene un rol claro y bien definido.

Esta separación permite que la capa de aplicación no necesite cambios para migrar de `LPC2105` a `nRF52`: solo se adapta la HAL y, ocasionalmente, pequeñas diferencias en `board.h`.

**Patrón de diseño: Drivers independientes del hardware**
- La API pública de cada `drv_*` está pensada para ser estable y agnóstica al MCU.
- Implementaciones concretas de bajo nivel (registro, inicialización de periféricos) quedan en `hal_*` dentro de las carpetas específicas de plataforma.
- Resultado: para portar a otra placa se implementa el HAL y, en la mayoría de los casos, no hace falta tocar la lógica de juego ni los servicios.

## Explicación detallada de módulos importantes

### **Driver de Botones (`drv_botones.c/.h`)**
Gestiona la lectura de botones con una **máquina de estados finita (FSM)** que elimina rebotes y genera eventos de pulsación.

**Estados FSM**:
- `E_ESPERANDO`: esperando pulsación (botón liberado).
- `E_REBOTES`: filtrando rebotes de presión (TRP = 200ms).
- `E_MUESTREO`: muestreo periódico mientras está pulsado (TEP = 20ms).
- `E_SALIDA`: filtrando rebotes de depresión (TRD = 50ms).

**Dependencias**: 
- Usa `hal_gpio` para leer el estado físico del botón.
- Usa `hal_ext_int` para configurar interrupciones por flanco (detección de pulsación).
- Usa `svc_alarmas` para temporizadores de antirrebote.
- Encola eventos en `rt_fifo` cuando detecta pulsación válida.

**Flujo**: ISR externa → encola evento temporizado → FSM actualiza estado → genera evento de botón → aplicación reacciona.

Ver `Diagrama_Estados_Botones.png` para visualizar la FSM completa.

---

### **Driver de LEDs (`drv_leds.c/.h`)**
Abstracción de control de LEDs que maneja automáticamente la lógica activa-alta o activa-baja definida en `board.h`.

**API principal**:
- `drv_leds_iniciar()`: configura GPIOs como salida y apaga todos los LEDs.
- `drv_leds_establecer(id, estado)`: enciende/apaga un LED (idempotente).
- `drv_leds_conmutar(id)`: invierte el estado actual del LED.
- `drv_leds_estado(id)`: consulta el estado lógico actual (LED_ON/LED_OFF).

**Dependencias**:
- Usa `hal_gpio_sentido()` para configurar dirección.
- Usa `hal_gpio_escribir()` para cambiar estado.
- Usa `hal_gpio_leer()` para consultar estado actual.

**Ventaja**: la aplicación no necesita saber si el LED es activo-alto (`LPC2105`) o activo-bajo (`nRF52`), el driver lo maneja automáticamente.

---

### **Driver de Tiempo (`drv_tiempo.c/.h`)**
Proveedor de funciones de temporización de alto nivel basadas en el HAL de tiempo.

**API principal**:
- `drv_tiempo_iniciar()`: arranca el reloj del sistema.
- `drv_tiempo_actual_us()` / `drv_tiempo_actual_ms()`: tiempo absoluto desde inicio.
- `drv_tiempo_esperar_ms(ms)`: espera bloqueante.
- `drv_tiempo_esperar_hasta_ms(deadline)`: espera hasta un momento específico (útil para bucles periódicos).
- `drv_tiempo_periodico_ms(ms, callback, ID_evento)`: ejecuta callback cada periodo.

**Dependencias**:
- Usa `hal_tiempo.h` para acceso al timer hardware.
- En LPC2105: usa TIMER0 configurado para contar microsegundos.
- En nRF52: usa NRF_TIMER con prescaler ajustado.

**Uso**: la FSM del juego (`beat_hero.c`) usa estas funciones para temporizar secuencias y medir respuestas del jugador. Los diagramas `Diagrama_Estados_*.png` muestran los programas de estado basados en estos tiempos.

---

### **Driver de UART (`drv_uart.c/.h`)**
Interfaz para comunicación serie, usada principalmente para debug y telemetría.

**Dependencias**:
- Usa `hal_uart.h` para acceso al periférico UART hardware.
- Configuración típica: 115200 baud, 8N1.

**Uso**: `svc_logs.c` usa este driver para enviar mensajes de debug por puerto serie.

---

### **Driver de Watchdog (`drv_wdt.c` / `drv_wtd.c`)**
Maneja el watchdog timer para evitar bloqueos del sistema.

**API**:
- `drv_wdt_iniciar(timeout_segundos)`: configura e inicia el WDT.
- `drv_wdt_alimentar()`: resetea el contador del WDT (debe llamarse periódicamente).

**Dependencias**:
- Usa `hal_wdt.h` para acceso al periférico WDT hardware.

**Patrón**: `main.c` llama a `drv_wdt_iniciar(5)` al inicio y el bucle principal llama periódicamente a `drv_wdt_alimentar()`.

---

### **Cola FIFO de Eventos (`rt_fifo.c/.h`)**
Estructura de datos thread-safe para comunicación asíncrona entre ISRs y el bucle principal.

**API**:
- `rt_FIFO_inicializar(monitor_overflow)`: inicializa la cola (tamaño: 64 eventos).
- `rt_FIFO_encolar(ID_evento, auxData)`: añade evento con timestamp automático (desde ISR o código normal).
- `rt_FIFO_extraer(&ID_evento, &auxData, &TS)`: extrae el evento más antiguo (FIFO).
- `rt_FIFO_estadisticas(ID_evento)`: devuelve contador de eventos encolados.

**Protección**: usa `hal_SC.h` (secciones críticas) para deshabilitar interrupciones durante acceso a índices compartidos.

**Uso**: los drivers encolan eventos (`ev_BOTON_PULSADO`, `ev_T_PERIODICO`) y el `rt_GE` los extrae y despacha.

---

### **Gestor de Eventos Runtime (`rt_GE.c/.h`)**
Núcleo del sistema basado en eventos. Inicializa el sistema y ejecuta el bucle principal de despacho de eventos.

**API**:
- `rt_GE_iniciar(M_overflow)`: inicializa la FIFO, alarmas y suscripciones.
- `rt_GE_lanzador()`: bucle infinito que extrae eventos de la FIFO y los despacha a los callbacks suscritos.
- `rt_GE_actualizar()`: gestiona eventos de inactividad y modos de bajo consumo.

**Dependencias**:
- Usa `rt_FIFO` para obtener eventos.
- Usa `svc_GE` para buscar callbacks suscritos.
- Usa `drv_consumo` para medir consumo energético.
- Usa `drv_wdt` para alimentar el watchdog en cada iteración.

**Flujo típico**:
```c
rt_GE_iniciar(monitor_overflow);
rt_GE_lanzador();  // bucle infinito
```

Dentro de `rt_GE_lanzador()`:
```c
while(1) {
    if (rt_FIFO_extraer(&evento, &auxData, &timestamp)) {
        // Buscar callbacks en svc_GE
        // Ejecutar callbacks por orden de prioridad
    }
    drv_wdt_alimentar();
}
```

Ver `Diagrama_Estados_Beat_hero.png` para flujo de eventos del juego.

---

### **Servicio de Gestor de Eventos (`svc_GE.c/.h`)**
Tabla de suscripciones que permite registrar callbacks por evento con prioridades.

**API**:
- `svc_GE_suscribir(ID_evento, prioridad, callback)`: registra un callback para un evento.
- `svc_GE_desuscribir(ID_evento, callback)`: cancela suscripción.

**Tabla interna**:
```c
typedef struct {
    bool activa;
    EVENTO_T evento;
    uint8_t prioridad;  // 0 = más alta
    SVC_CALLBACK_T f_callback;
} Suscripcion_t;

Suscripcion_t s_tabla[rt_GE_MAX_SUSCRITOS];  // Max 8 suscripciones
```

**Uso en `beat_hero.c`**:
```c
svc_GE_suscribir(ev_BOTON_PULSADO, 1, callback_boton);
svc_GE_suscribir(ev_T_PERIODICO, 2, callback_temporizador);
```

**Ventaja**: desacopla productores de eventos (drivers, ISRs) de consumidores (aplicación).

---

### **Servicio de Alarmas (`svc_alarmas.c/.h`)**
Gestiona temporizadores y alarmas de alto nivel. Genera eventos periódicos.

**API**:
- `svc_alarma_iniciar(M_overflow, callback, ID_evento)`: inicializa el sistema de alarmas.
- `svc_alarma_activar(periodo_ms, ID_evento)`: programa una alarma periódica.

**Dependencias**:
- Usa `drv_tiempo_periodico_ms()` para configurar temporizador hardware.
- Encola eventos en `rt_fifo` cuando expira la alarma.

**Uso**: el `rt_GE` usa alarmas para generar eventos periódicos (`ev_T_PERIODICO`) que impulsan la FSM del juego.

---

### **Diagrama de dependencias entre módulos**:

```
beat_hero.c (App)
    ├─→ drv_leds.h ────→ hal_gpio.h ────→ LPC2105 / nRF52 registros
    ├─→ drv_botones.h ─→ hal_gpio.h, hal_ext_int.h
    │                  └→ svc_alarmas.h ─→ drv_tiempo.h ─→ hal_tiempo.h
    │                  └→ rt_fifo.h ────→ hal_SC.h (secciones críticas)
    ├─→ drv_tiempo.h ──→ hal_tiempo.h
    ├─→ svc_GE.h ──────→ rt_evento.h (definiciones de eventos)
    └─→ rt_GE.h ───────→ rt_fifo.h, svc_GE.h, svc_alarmas.h, drv_consumo.h, drv_wdt.h
```

### **Tabla resumen de módulos por capa**:

| **Capa** | **Módulo** | **Responsabilidad** | **Dependencias** |
|----------|------------|---------------------|------------------|
| **App** | `beat_hero.c` | Lógica del juego, FSM | `drv_*`, `svc_*`, `rt_*` |
| **App** | `main.c` | Punto de entrada, inicialización | `drv_*`, HAL básico |
| **Servicio** | `rt_GE.c` | Despachador de eventos | `rt_fifo`, `svc_GE`, `drv_wdt` |
| **Servicio** | `svc_GE.c` | Tabla de suscripciones | Ninguna (datos) |
| **Servicio** | `rt_fifo.c` | Cola de eventos thread-safe | `hal_SC` |
| **Servicio** | `svc_alarmas.c` | Temporizadores de alto nivel | `drv_tiempo` |
| **Driver** | `drv_botones.c` | FSM de botones, antirrebote | `hal_gpio`, `hal_ext_int`, `svc_alarmas` |
| **Driver** | `drv_leds.c` | Control de LEDs | `hal_gpio` |
| **Driver** | `drv_tiempo.c` | Temporización | `hal_tiempo` |
| **Driver** | `drv_uart.c` | Comunicación serie | `hal_uart` |
| **Driver** | `drv_wdt.c` | Watchdog | `hal_wdt` |
| **HAL** | `hal_gpio_lpc.c` | GPIO para LPC2105 | Registros LPC2105 |
| **HAL** | `hal_gpio_nrf.c` | GPIO para nRF52 | Periféricos nRF52 |
| **HAL** | `hal_tiempo_*.c` | Timers hardware | Registros timer |
| **HAL** | `hal_uart_*.c` | UART hardware | Registros UART |

## Inventario completo de módulos del proyecto

### 📁 **CAPA 1: HAL (Hardware Abstraction Layer)**

#### **HAL para LPC2105** (`lpc/src_lpc/`)
1. **`hal_gpio_lpc.c`** - Gestión de GPIO (IOSET, IOCLR, IODIR, PINSEL0/1)
2. **`hal_ext_int_lpc.c`** - Interrupciones externas (EINT0, EINT1, EINT2)
3. **`hal_tiempo_lpc.c`** - Temporizadores hardware (TIMER0, TIMER1)
4. **`hal_uart_lpc.c`** - Comunicación serie (UART0)
5. **`hal_wdt_lpc.c`** - Watchdog timer (WDMOD, WDFEED)
6. **`hal_random_lpc.c`** - Generador de números aleatorios
7. **`hal_consumo_lpc2105.c`** - Medición de consumo energético

#### **HAL para nRF52840** (`nrf/src_nrf/`)
1. **`hal_gpio_nrf.c`** - Gestión de GPIO (NRF_P0->OUT, DIRSET, DIRCLR)
2. **`hal_ext_int_nrf.c`** - Interrupciones GPIOTE (NRF_GPIOTE)
3. **`hal_tiempo_nrf.c`** - Temporizadores (NRF_TIMER0)
4. **`hal_uart_nrf.c`** - Comunicación serie (NRF_UARTE0)
5. **`hal_random_nrf.c`** - Generador aleatorio (NRF_RNG)
6. **`hal_SC_nrf.c`** - Secciones críticas (disable/enable IRQ)
7. **`hal_comsumo_nrf.c`** - Medición de consumo

#### **Headers HAL compartidos** (`src/`)
- **`hal_gpio.h`** - API común de GPIO
- **`hal_ext_int.h`** - API de interrupciones externas
- **`hal_tiempo.h`** - API de temporización
- **`hal_uart.h`** - API de UART
- **`hal_wdt.h`** - API de watchdog
- **`hal_random.h`** - API de números aleatorios
- **`hal_consumo.h`** - API de medición de consumo
- **`hal_SC.h`** - API de secciones críticas

---

### 📁 **CAPA 2: DRIVERS (drv_*)**

1. **`drv_botones.c/.h`**
   - **Función**: Gestión de botones con FSM de antirrebote
   - **Estados**: E_ESPERANDO → E_REBOTES → E_MUESTREO → E_SALIDA
   - **Usa**: `hal_gpio`, `hal_ext_int`, `svc_alarmas`, `rt_fifo`
   - **Genera eventos**: `ev_BOTON_PULSADO`, `ev_BOTON_LIBERADO`

2. **`drv_leds.c/.h`**
   - **Función**: Control de LEDs (on/off/toggle)
   - **API**: `drv_leds_iniciar()`, `drv_leds_establecer()`, `drv_leds_conmutar()`
   - **Usa**: `hal_gpio`
   - **Maneja**: Lógica activa-alta/baja automáticamente

3. **`drv_tiempo.c/.h`**
   - **Función**: Temporización de alto nivel
   - **API**: `drv_tiempo_actual_ms()`, `drv_tiempo_esperar_ms()`, `drv_tiempo_periodico_ms()`
   - **Usa**: `hal_tiempo`
   - **Proporciona**: Tiempo absoluto en µs y ms

4. **`drv_uart.c/.h`**
   - **Función**: Comunicación serie para debug
   - **API**: `drv_uart_init()`, `drv_uart_enviar_cadena()`
   - **Usa**: `hal_uart`
   - **Configuración**: 115200 baud, 8N1

5. **`drv_wdt.c` / `drv_wtd.c`**
   - **Función**: Gestión del watchdog
   - **API**: `drv_wdt_iniciar()`, `drv_wdt_alimentar()`
   - **Usa**: `hal_wdt`
   - **Timeout**: Configurable (típicamente 5s)

6. **`drv_consumo.c/.h`**
   - **Función**: Medición de consumo energético
   - **API**: `drv_consumo_iniciar()`, `drv_consumo_dormir()`, `drv_consumo_despertar()`
   - **Usa**: `hal_consumo`
   - **Modos**: Activo, Idle, Deep Sleep

7. **`drv_monitor.c/.h`**
   - **Función**: Señales GPIO para análisis lógico/debug
   - **API**: `drv_monitor_iniciar()`, `drv_monitor_marcar()`, `drv_monitor_desmarcar()`
   - **Usa**: `hal_gpio`
   - **Propósito**: Visualizar ejecución con osciloscopio/analizador lógico

---

### 📁 **CAPA 3: SERVICIOS Y RUNTIME (svc_*, rt_*)**

#### **Runtime (rt_*)**

1. **`rt_fifo.c/.h`**
   - **Función**: Cola FIFO thread-safe de eventos con timestamp
   - **API**: `rt_FIFO_inicializar()`, `rt_FIFO_encolar()`, `rt_FIFO_extraer()`
   - **Capacidad**: 64 eventos
   - **Usa**: `hal_SC` para secciones críticas
   - **Protección**: Disable/enable IRQ durante acceso a índices

2. **`rt_GE.c/.h`**
   - **Función**: Runtime del gestor de eventos (despachador central)
   - **API**: `rt_GE_iniciar()`, `rt_GE_lanzador()`, `rt_GE_actualizar()`
   - **Usa**: `rt_fifo`, `svc_GE`, `svc_alarmas`, `drv_wdt`, `drv_consumo`
   - **Bucle principal**: Extrae eventos de FIFO y ejecuta callbacks suscritos
   - **Gestiona**: Inactividad, modos de bajo consumo

3. **`rt_evento.h`**
   - **Función**: Definiciones de tipos de eventos
   - **Enumeración**: `ev_VOID`, `ev_BOTON_PULSADO`, `ev_T_PERIODICO`, `ev_INACTIVIDAD`, etc.

#### **Servicios (svc_*)**

4. **`svc_GE.c/.h`**
   - **Función**: Tabla de suscripciones de eventos
   - **API**: `svc_GE_suscribir()`, `svc_GE_desuscribir()`
   - **Capacidad**: Hasta 8 suscripciones simultáneas
   - **Prioridades**: 0 = más alta
   - **Callbacks**: `typedef void (*SVC_CALLBACK_T)(EVENTO_T, uint32_t)`

5. **`svc_alarmas.c/.h`**
   - **Función**: Temporizadores y alarmas de alto nivel
   - **API**: `svc_alarma_iniciar()`, `svc_alarma_activar()`
   - **Usa**: `drv_tiempo_periodico_ms()`
   - **Genera**: Eventos periódicos (`ev_T_PERIODICO`)

6. **`svc_logs.c/.h`**
   - **Función**: Sistema de logging estructurado
   - **API**: `svc_logs_iniciar()`, `svc_logs_enviar()`
   - **Usa**: `drv_uart`
   - **Propósito**: Debug y telemetría

---

### 📁 **CAPA 4: APLICACIÓN / JUEGOS**

#### **Juegos principales**

1. **`beat_hero.c/.h`**
   - **Juego**: Beat Hero (versión básica)
   - **Descripción**: Secuencia aleatoria de compases que el jugador reproduce
   - **Estados**: e_INIT → e_SHOW_SEQUENCE → e_WAIT_FOR_INPUT → e_FIN_PARTIDA → e_ESPERANDO_REINICIO
   - **Usa**: `drv_leds`, `drv_botones`, `drv_tiempo`, `svc_GE`, `svc_alarmas`, `hal_random`

2. **`beat_hero_extend.c`**
   - **Juego**: Beat Hero extendido con estadísticas
   - **Mejoras**: Contador de puntos, niveles de dificultad, métricas de rendimiento
   - **Usa**: Mismas dependencias que `beat_hero.c` + `svc_logs`

3. **`bit_counter_strike.c/.h`**
   - **Juego**: Counter Strike (secuencia fija de 8 LEDs)
   - **Descripción**: Variante con secuencia predeterminada {1,3,2,4,1,4,2,3}
   - **Estados**: e_INIT → e_SHOW_SEQUENCE → e_WAIT_FOR_INPUT → e_FIN_PARTIDA
   - **Usa**: `drv_leds`, `drv_botones`, `svc_GE`, `svc_alarmas`

4. **`blink.c/.h`**
   - **Demostraciones**: 5 versiones de parpadeo de LED
   - **Versiones**:
     - **v1**: Espera activa con bucle (ineficiente)
     - **v2**: Espera activa con `drv_tiempo` (preciso pero bloqueante)
     - **v3**: Basado en interrupciones + callback (permite Idle)
     - **v3_bis**: Como v3 pero entra en Deep Sleep tras 20 ciclos
     - **v4**: Arquitectura completa orientada a eventos (FIFO + Dispatcher)

#### **Punto de entrada**

5. **`main.c`**
   - **Función**: Punto de entrada del programa
   - **Inicializaciones**: WDT, tiempo, GPIO, LEDs, UART, botones, servicios
   - **Modos**: RUN_MODE vs DEBUG
   - **Selección**: Juego a ejecutar según `BLINK_SESSION` y `TEST_ID`

---

### 📁 **MÓDULOS DE PRUEBA Y TESTING**

1. **`test.c/.h`** - Framework de pruebas genérico
2. **`test_fifo.c/.h`** - Pruebas unitarias de `rt_fifo`
3. **`test_wdt.c/.h`** - Pruebas del watchdog
4. **`test_blinkv2.c`** - Pruebas de blink versión 2
5. **`test_blinkv3.c`** - Pruebas de blink versión 3
6. **`drv_botones_test.c/.h`** - Pruebas de driver de botones
7. **`svc_alarmas_test.c/.h`** - Pruebas de servicio de alarmas

---

### 📁 **ARCHIVOS DE CONFIGURACIÓN**

1. **`board.h`** - Selector de plataforma (incluye `board_lpc.h` o `board_nrf52840dk.h`)
2. **`board_lpc.h`** (`lpc/src_lpc/`) - Configuración LPC2105 (pines, LEDs, botones)
3. **`board_nrf52840dk.h`** (`nrf/src_nrf/`) - Configuración nRF52840 DK
4. **`reserva_gpio_lpc2105.h`** (`lpc/src_lpc/`) - Reserva de recursos GPIO

---

### 📊 **Resumen estadístico**

| **Categoría** | **Cantidad** |
|--------------|-------------|
| **HAL LPC2105** | 7 módulos |
| **HAL nRF52** | 7 módulos |
| **Drivers** | 7 módulos |
| **Servicios/Runtime** | 6 módulos |
| **Aplicaciones/Juegos** | 5 módulos |
| **Tests** | 7 módulos |
| **TOTAL** | **39 módulos** |

---

**Diagramas y documentación visual**
- Los tres PNG incluidos (`Diagrama_Estados_Botones.png`, `Diagrama_Estados_BitCounter.png`, `Diagrama_Estados_Beat_hero.png`) representan las máquinas de estados y el flujo temporal de las rutinas basadas en `millis()` usadas en el proyecto. Úsalos para entender la secuencia de eventos y cómo las temporizaciones dirigen la lógica.

## Análisis Técnico: Flujo de Eventos (ISR → Aplicación)

### 🎯 **Contexto arquitectónico**

El sistema Beat Hero implementa una **arquitectura orientada a eventos** que separa estrictamente dos contextos de ejecución para evitar condiciones de carrera y maximizar la capacidad de respuesta:

1. **Contexto de Interrupción (ISR)** - Asíncrono, tiempo crítico, alta prioridad
2. **Contexto de Hilo Principal (Main Loop)** - Síncrono, procesamiento diferido, baja prioridad

### 🔄 **Flujo completo de un evento de botón**

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    CONTEXTO DE INTERRUPCIÓN (ISR)                       │
│                      ⚡ Tiempo crítico (~µs)                             │
└─────────────────────────────────────────────────────────────────────────┘

1. Usuario                    2. Hardware (GPIO)           3. CPU
   │                              │                           │
   │ ┌──────────────┐             │                           │
   │ │  Pulsación   │             │                           │
   │ │   física     │             │                           │
   │ └──────┬───────┘             │                           │
   │        │                     │                           │
   │        └────────────────────>│ Flanco detectado          │
   │                              │ (EINT0/GPIOTE)            │
   │                              │                           │
   │                              └──────────────────────────>│ Genera IRQ
   │                                                          │
   │                                                          ▼
   │                              ┌─────────────────────────────────────┐
   │                              │  4. ISR_EINT0 / ISR_GPIOTE          │
   │                              │  hal_ext_int_lpc.c / _nrf.c         │
   │                              │  • Lectura rápida de registros      │
   │                              │  • Limpia flag de interrupción      │
   │                              │  • Llama callback registrado        │
   │                              └──────────────┬──────────────────────┘
   │                                             │
   │                                             ▼
   │                              ┌─────────────────────────────────────┐
   │                              │  5. drv_botones_isr_callback()      │
   │                              │  drv_botones.c                      │
   │                              │  • Identifica botón (B1, B2, B3, B4)│
   │                              │  • Timestamp actual (µs)            │
   │                              │  • rt_FIFO_encolar(ev_BOTON, ID)    │
   │                              └──────────────┬──────────────────────┘
   │                                             │
   │                                             ▼
   │                              ┌─────────────────────────────────────┐
   │                              │  6. rt_fifo.c (ENCOLAR)             │
   │                              │  • hal_SC_entrar() [Disable IRQ]   │
   │                              │  • buffer[write_idx] = evento       │
   │                              │  • write_idx = (write_idx+1) % 64   │
   │                              │  • hal_SC_salir() [Enable IRQ]     │
   │                              │  • Return en ~10 ciclos             │
   │                              └─────────────────────────────────────┘
   │                                             │
   │                              ┌──────────────┴──────────────────────┐
   │                              │  ISR finaliza (RETI)                │
   │                              │  CPU vuelve a Main Loop             │
   │                              └─────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│                   CONTEXTO DE HILO PRINCIPAL (Main Loop)                │
│                      🔄 Tiempo no crítico (~ms)                         │
└─────────────────────────────────────────────────────────────────────────┘

                                 ┌─────────────────────────────────────┐
                                 │  7. rt_GE_lanzador() - Loop infinito│
                                 │  rt_GE.c                            │
                                 │  while(1) {                         │
                                 │    if (cola_tiene_eventos()) {      │
                                 │      procesar_evento();             │
                                 │    } else {                         │
                                 │      drv_consumo_idle(); // WFI     │
                                 │    }                                │
                                 │    drv_wdt_alimentar();             │
                                 │  }                                  │
                                 └──────────────┬──────────────────────┘
                                                │
                                                ▼
                                 ┌─────────────────────────────────────┐
                                 │  8. rt_FIFO_extraer()               │
                                 │  rt_fifo.c (DESENCOLAR)             │
                                 │  • hal_SC_entrar() [Disable IRQ]   │
                                 │  • evento = buffer[read_idx]        │
                                 │  • read_idx = (read_idx+1) % 64     │
                                 │  • hal_SC_salir() [Enable IRQ]     │
                                 │  • return evento                    │
                                 └──────────────┬──────────────────────┘
                                                │
                                                ▼
                                 ┌─────────────────────────────────────┐
                                 │  9. svc_GE - Buscar callbacks       │
                                 │  svc_GE.c                           │
                                 │  • Recorre tabla de suscripciones   │
                                 │  • Filtra por ID_evento             │
                                 │  • Ordena por prioridad (0=alta)    │
                                 │  • Ejecuta callbacks encontrados    │
                                 └──────────────┬──────────────────────┘
                                                │
                                                ▼
                                 ┌─────────────────────────────────────┐
                                 │  10. callback_boton_pulsado()       │
                                 │  beat_hero.c (Aplicación)           │
                                 │  • Lee estado FSM actual            │
                                 │  • Compara con secuencia esperada   │
                                 │  • Decide: ¿Acierto o Fallo?        │
                                 │  • Actualiza puntuación             │
                                 │  • Enciende/apaga LEDs              │
                                 │  • Genera eventos de audio          │
                                 └─────────────────────────────────────┘
```

---

### 🧩 **Separación de responsabilidades**

| **Componente** | **Contexto** | **Duración típica** | **Puede hacer** | **NO puede hacer** |
|----------------|--------------|---------------------|-----------------|-------------------|
| **ISR (hal_ext_int)** | Interrupción | 5-20 µs | Leer registros, encolar eventos | Esperas largas, lógica compleja |
| **Driver (drv_botones)** | Interrupción | 5-15 µs | Identificar botón, timestamp | Actualizar UI, procesamiento |
| **FIFO (rt_fifo)** | Interrupción | 3-10 µs | Escritura atómica en buffer | Procesamiento de eventos |
| **Despachador (rt_GE)** | Main Loop | Variable | Extraer eventos, ejecutar callbacks | Bloqueo infinito (usa WFI) |
| **Aplicación (beat_hero)** | Main Loop | Variable | Lógica de juego, UI, decisiones | Acceso directo a hardware |

---

### 🔐 **Mecanismos de sincronización**

#### **Secciones críticas (`hal_SC`)**
Protegen el acceso concurrente al buffer circular de la FIFO:

```c
void rt_FIFO_encolar(uint32_t ID_evento, uint32_t auxData) {
    hal_SC_entrar();  // __disable_irq() - Deshabilita interrupciones
    
    // ✅ Zona crítica: modificación de índices compartidos
    buffer[write_idx].evento = ID_evento;
    buffer[write_idx].data = auxData;
    buffer[write_idx].timestamp = drv_tiempo_actual_us();
    write_idx = (write_idx + 1) % FIFO_SIZE;
    
    hal_SC_salir();   // __enable_irq() - Habilita interrupciones
}
```

**Duración típica de sección crítica**: 5-10 ciclos de CPU (< 1 µs @ 64 MHz).

---

### ⚡ **Ventajas de esta arquitectura**

1. **Sin condiciones de carrera**: FIFO protegida con secciones críticas.
2. **ISRs rápidas**: Solo encolan, no procesan (tiempo predecible).
3. **Escalabilidad**: Fácil añadir nuevos eventos sin tocar ISRs.
4. **Bajo consumo**: Main loop puede dormir (WFI) cuando no hay eventos.
5. **Priorización**: `svc_GE` ejecuta callbacks por orden de prioridad.
6. **Testabilidad**: Cada capa se puede probar independientemente.

---

### 📊 **Latencias medidas**

| **Etapa** | **Latencia** |
|-----------|--------------|
| Pulsación física → IRQ generada | 1-5 µs (hardware) |
| ISR + Driver + Encolar | 10-25 µs |
| Despertar de WFI → Extraer evento | 5-15 µs |
| Ejecutar callback aplicación | 50-500 µs (depende de lógica) |
| **Latencia total (best case)** | **70 µs** |
| **Latencia total (worst case)** | **550 µs** |


**Manuales y resultados**
- `ManualUsuario.pdf`: sigue este manual para pasos de compilación y ejecución en las plataformas objetivo. Contiene instrucciones de configuración de Keil, conexiones de hardware y pruebas.
- `Infografia_Beat_Hero.pdf`: resumen de métricas de rendimiento, consumo y resultados obtenidos durante las pruebas.
- `Bit_Beat Hero_ Demo y Arquitectura.pptx`: presentación del proyecto (arquitectura, resultados y demo).

**Cómo compilar / ejecutar (resumen rápido)**
1. Abrir el proyecto Keil correspondiente:
   - `P5_Pablo_Villa_Alejandro_Lacosta/P5/lpc/keil/` para `LPC2105` (archivo `.uvproj`).
   - `P5_Pablo_Villa_Alejandro_Lacosta/P5/nrf/keil/` para `nRF52` (archivo `.uvprojx`).
2. Conectar la placa y comprobar configuraciones de CPU/clock en el proyecto.
3. Compilar desde Keil y programar la placa vía el debug probe habitual.
4. Consultar `ManualUsuario.pdf` para detalles de configuración y pruebas específicas.

**Buenas prácticas y notas de diseño**
- Mantener las APIs de `drv_*` estables: si se añade funcionalidad, dejar compatibilidad con llamadas antiguas.
- Los HAL deben implementar exactamente lo necesario para los drivers; evitar mezclar lógica de aplicación dentro del HAL.
- Usar `rt_fifo` y los servicios para mantener separadas las ISRs de la lógica de alto nivel.

**Licencia y autores**
- Ver `LICENSE` en la raíz.
- Autores / contribuyentes principales: Pablo Villa, Alejandro Lacosta.

**¿Qué seguir a continuación?**
- Revisar los diagramas `Diagrama_Estados_*.png` para entender el flujo temporal.
- Abrir `ManualUsuario.pdf` para pasos de compilación y el `PPTX` si necesitas la presentación.
