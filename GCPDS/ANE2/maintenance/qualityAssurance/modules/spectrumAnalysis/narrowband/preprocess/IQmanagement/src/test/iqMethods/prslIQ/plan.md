Considera que en el directorio actual existen las carpetas `docs/` y `src/`, donde encontrarás documentación, métricas y evidencia técnica relacionadas con la optimización de un algoritmo de calibración IQ implementado inicialmente en Python y posteriormente optimizado en C.

El proyecto ya incluye:
- Métricas comparativas entre Python y C.
- Optimización de procesamiento de I/O de datos.
- Optimización de cálculo de espectro de potencia.
- Optimización de KDE (Kernel Density Estimation).
- Métricas de rendimiento, latencia y uso de recursos.
- Objetivo final de despliegue sobre Raspberry Pi como nodo sensor SDR.

Dentro de `src/` encontrarás tanto las implementaciones en Python como en C utilizadas durante el proceso.

Quiero que analices especialmente la implementación en C y su estructura actual de directorios, incluyendo:
- `include/`
- `src/`
- `metrics/`
- `docs/`
- scripts existentes
- flujo de compilación y ejecución

Con base en esa estructura, quiero que implementes COMPLETAMENTE DESDE CERO dentro de una nueva carpeta llamada `liquidSDR/` una nueva versión del pipeline usando la librería LiquidDSP/LiquidSDR:

https://liquidsdr.org/

Especialmente las herramientas relacionadas con FFT/Fourier:
https://liquidsdr.org/doc/fft/

Y cualquier otro módulo de LiquidDSP que consideres útil para:
- procesamiento SDR
- espectro de potencia
- filtrado
- procesamiento IQ
- optimización FFT
- análisis estadístico
- manejo eficiente de buffers
- streaming de datos
- reducción de latencia
- procesamiento embebido para Raspberry Pi

Objetivos principales:
1. Reestructurar el proyecto profesionalmente usando LiquidDSP.
2. Mantener una arquitectura modular y portable.
3. Mejorar aún más el rendimiento respecto a la versión actual en C.
4. Preparar el proyecto específicamente para Raspberry Pi como nodo SDR embebido.
5. Mantener trazabilidad y evidencia metodológica del proceso.

La nueva carpeta `liquidSDR/` debe incluir:
- `include/`
- `src/`
- `metrics/`
- `docs/`
- `scripts/`
- `build/`
- `data/`
- `results/`
- `web/` (para visualización HTML)
- `tests/`

Quiero además que implementes:

1. Scripts automatizados:
- `build_and_run.sh`
- `benchmark.sh`
- `collect_metrics.sh`
- `deploy_rpi.sh`

Estos scripts deben:
- compilar automáticamente
- ejecutar benchmarks
- recolectar métricas
- generar resultados
- exportar logs
- generar reportes

2. Sistema de métricas:
Implementa métricas comparativas para:
- tiempo de ejecución
- uso de memoria
- throughput
- latencia
- FFT timing
- velocidad de I/O
- uso de CPU
- eficiencia energética estimada
- rendimiento Raspberry Pi vs PC

3. Visualización HTML:
Genera una pequeña página HTML automática dentro de `web/` que:
- cargue automáticamente los resultados generados
- visualice gráficas y tablas
- permita validar rápidamente las métricas
- muestre comparativas Python vs C vs LiquidDSP
- sea fácil de abrir localmente después de ejecutar `build_and_run.sh`

La página debe generarse automáticamente tras la ejecución.

4. Documentación metodológica:
Dentro del directorio raíz de `prslIQ/`, crea un archivo `.md` detallando:
- arquitectura del sistema
- flujo de despliegue
- cross-compilation
- compilación nativa para Raspberry Pi
- dependencias necesarias
- instalación de LiquidDSP
- optimizaciones aplicadas
- estructura del proyecto
- explicación técnica de FFT usada
- pipeline completo SDR
- despliegue del nodo sensor Raspberry Pi
- troubleshooting
- automatización de métricas
- cómo reproducir experimentos

IMPORTANTE:
- Mantén consistencia arquitectónica con el proyecto actual.
- Reutiliza ideas y métricas existentes.
- Prioriza código modular, portable y mantenible.
- Usa C moderno y buenas prácticas.
- Optimiza para bajo consumo y ejecución embebida.
- Documenta cada decisión técnica importante.
- Si el contexto es demasiado grande, trabaja por etapas sin perder coherencia arquitectónica.
- Antes de escribir código, analiza primero toda la estructura actual del proyecto.
- Genera primero un plan técnico detallado del refactor y migración.
- Luego implementa progresivamente cada módulo.