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

**Arquitectura por capas**
El proyecto sigue una arquitectura en capas para facilitar la reutilización y la independencia del hardware:

- **Capa HAL (Hardware Abstraction Layer)**: `hal_*.c/h` contienen las abstracciones de más bajo nivel (GPIO, UART, WDT, temporizadores). Cada plataforma (LPC, nRF) implementa su versión del HAL en `lpc/src_lpc` o `nrf/src_nrf`. La HAL es la única capa que interactúa directamente con los registros/periféricos del MCU.

- **Drivers (drv_*)**: sobre la HAL se sitúan los drivers que exponen APIs funcionales (por ejemplo, para botones, LEDs, watchdog, UART). Ejemplos:
  - `drv_botones.c/.h`: gestión de entrada por botones, debounce y mapeo a eventos del juego.
  - `drv_leds.c/.h`: abstrae la manipulación de LEDs del tablero/juego.
  - `drv_tiempo.c/.h`: funciones de temporización (millis, delays) usadas por el motor de estados.
  - `drv_uart.c/.h`: comunicación serie para debug/telemetría.
  - `drv_wdt.c` / `drv_wtd.c`: watchdog y control de reinicio.

- **Servicios / Middlewares (svc_*, rt_*)**: componentes reutilizables que implementan lógica de soporte:
  - `rt_fifo.c/.h`: colas FIFO para comunicación entre módulos.
  - `rt_GE.c/.h` y `svc_GE.c/.h`: gestor de eventos/estado del juego (máquina de estados que coordina niveles, puntos y transiciones).
  - `svc_alarmas.c/.h`: gestión de alarmas y temporizadores de alto nivel.
  - `svc_logs.c/.h`: registro de eventos y ayuda al debug.

- **Aplicación / Juego**: `beat_hero.c`, `beat_hero_extend.c`, `bit_counter_strike.c`, `main.c` contienen la lógica del juego (entrada, puntuación, ritmos) y usan los servicios y drivers para interactuar con el hardware.

Esta separación permite que la capa de aplicación no necesite cambios para cambiar de `LPC2105` a `nRF52`: solo se adapta la HAL y, cuando es necesario, pequeñas diferencias en los drivers.

**Patrón de diseño: Drivers independientes del hardware**
- La API pública de cada `drv_*` está pensada para ser estable y agnóstica al MCU.
- Implementaciones concretas de bajo nivel (registro, inicialización de periféricos) quedan en `hal_*` dentro de las carpetas específicas de plataforma.
- Resultado: para portar a otra placa se implementa el HAL y, en la mayoría de los casos, no hace falta tocar la lógica de juego ni los servicios.

**Explicación breve de módulos importantes**
- `drv_botones.*` — lectura y filtrado de pulsadores, generación de eventos (pulsado/largo) y mapeo a acciones del juego.
- `drv_leds.*` — control de patrones y brillo (si aplica) de los LEDs usados para feedback en el juego.
- `drv_tiempo.*` — proveedor de `millis()` y funciones de temporización no bloqueantes para la máquina de estados. Los diagramas `Diagrama_Estados_*.png` muestran los programas de estado basados en estos tiempos.
- `drv_uart.*` — interfaz para depuración y mensajes de estado.
- `drv_wdt.*` — manejo del watchdog para evitar bloqueos en la ejecución embebida.
- `rt_fifo.*` — estructura de datos para intercambio seguro de mensajes entre ISR y tareas principales.
- `svc_GE.*` / `rt_GE.*` — núcleo de la máquina de estados del juego: gestiona niveles, entradas sincronizadas con la música y puntuaciones.

**Diagramas y documentación visual**
- Los tres PNG incluidos (`Diagrama_Estados_Botones.png`, `Diagrama_Estados_BitCounter.png`, `Diagrama_Estados_Beat_hero.png`) representan las máquinas de estados y el flujo temporal de las rutinas basadas en `millis()` usadas en el proyecto. Úsalos para entender la secuencia de eventos y cómo las temporizaciones dirigen la lógica.

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
- Autores / contribuyentes principales: Pablo Villa, Alejandro Lacosta (y otros colaboradores dentro de la carpeta del P5).

**¿Qué seguir a continuación?**
- Revisar los diagramas `Diagrama_Estados_*.png` para entender el flujo temporal.
- Abrir `ManualUsuario.pdf` para pasos de compilación y el `PPTX` si necesitas la presentación para exponer el proyecto.
