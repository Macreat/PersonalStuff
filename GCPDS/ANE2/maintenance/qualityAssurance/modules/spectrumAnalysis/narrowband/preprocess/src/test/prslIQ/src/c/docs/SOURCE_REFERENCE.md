# Documentacion tecnica del codigo C

## Objetivo
Este documento describe de forma minimalista y profesional la arquitectura, todos los modulos y todas las funciones implementadas en el proyecto C.

## Arquitectura de alto nivel
Pipeline de procesamiento:
1. Descubrimiento de pares SigMF
2. Lectura de metadata
3. Carga IQ int8 (chunked y acotada)
4. Conversion a complejo float
5. Preproceso Metodo 3 (DC + RMS)
6. Welch PSD
7. Metricas y reporte (terminal/JSON/CSV)

## Flujo de control (main)
1. Parsear argumentos CLI y validar opciones.
2. Descubrir pares `.sigmf-data/.sigmf-meta`.
3. Inicializar tracker de memoria y workspace Welch.
4. Reutilizar buffers `raw_buf` e `iq_buf` por archivo.
5. Ejecutar pipeline por archivo y acumular metricas.
6. Calcular estadisticos globales (SNR mean/std, jitter, throughput, score).
7. Emitir resumen por terminal y opcionalmente JSON/CSV.
8. Liberar recursos y verificar consistencia del tracker.

## Referencia por archivo

### include/iq_bench.h
Rol:
- Define tipos de datos compartidos (complejo, metadata, metricas, tiempos, memoria).
- Declara la API publica entre modulos.

Elementos clave:
- `complexf_t`, `sigmf_meta_t`, `iq_metrics_t`, `perf_timing_t`.
- `welch_workspace_t` para buffers reutilizables y estado de Welch.

Notas de ingenieria:
- Mantener este header estable permite refactorizar implementaciones sin romper el CLI.

Funciones expuestas:
- `now_ms`, `process_rss_mb`, `process_peak_rss_mb`, `system_ram_percent`
- `mem_tracker_init`, `mt_alloc`, `mt_free`
- `discover_sigmf_pairs`
- `parse_sigmf_meta`, `load_iq_int8_limited`, `load_iq_int8_limited_chunked_into`, `int8_to_complexf`
- `method3_dc_rms_norm`
- `welch_workspace_init`, `welch_workspace_free`, `welch_fft_shifted`, `compute_metrics_from_psd`

### src/main.c
Rol:
- Punto de entrada CLI.
- Orquesta el pipeline por archivo.
- Acumula estadisticas globales y emite resumen final.

Responsabilidades:
- Parseo de argumentos (`--db`, `--files`, `--max-complex`, `--nperseg`, `--overlap`, `--chunk-bytes`, `--json-out`, `--csv-out`).
- Reserva de buffers reutilizables.
- Manejo centralizado de tiempos por etapa.
- Control de errores por archivo y continuacion del benchmark.

Riesgos controlados:
- Cleanup unico para recursos globales.
- Verificacion de fugas con `mem_tracker_t`.

Catalogo de funciones:
- `print_usage(const char *exe)`
	- Imprime ayuda CLI y defaults de referencia.
- `write_json_summary(...)`
	- Escribe resumen consolidado de la corrida en JSON.
- `write_csv_summary(...)`
	- Escribe resumen consolidado de la corrida en CSV (1 fila).
- `main(int argc, char **argv)`
	- Orquesta todo el benchmark y controla codigos de salida.

### src/fs_utils.c
Rol:
- Descubre pares `.sigmf-data` y `.sigmf-meta` en directorio local.

Responsabilidades:
- Filtrado por sufijo.
- Verificacion de existencia del metadata asociado.
- Orden lexicografico estable para reproducibilidad.

Observacion:
- Implementacion enfocada en Windows (`FindFirstFileA`).

Catalogo de funciones:
- `has_suffix(const char *s, const char *suffix)`
	- Helper de filtrado por extension.
- `pair_cmp(const void *a, const void *b)`
	- Comparador para orden deterministico con `qsort`.
- `discover_sigmf_pairs(const char *db_dir, sigmf_pair_t *out_pairs, int max_pairs)`
	- Recorre directorio y devuelve pares validos data/meta.

### src/sigmf_io.c
Rol:
- Entrada/salida de SigMF y conversion de datos crudos.

Responsabilidades:
- Parseo de `core:sample_rate` y `core:frequency`.
- Carga limitada por maximo de muestras para benchmarking.
- Carga chunked para controlar memoria y I/O.
- Conversion `int8` intercalado a complejo float.

Mejora aplicada:
- En carga chunked se garantiza conteo de bytes par para evitar I/Q incompleto.

Catalogo de funciones:
- `extract_u64(const char *text, const char *key, uint64_t *out_value)`
	- Extrae valor numerico de clave textual en metadata.
- `parse_sigmf_meta(const char *meta_path, sigmf_meta_t *out_meta)`
	- Lee metadata y completa frecuencia central y sample rate.
- `load_iq_int8_limited(...)`
	- Carga IQ acotado por limite maximo y reserva con tracker.
- `load_iq_int8_limited_chunked_into(...)`
	- Carga por bloques en buffer reutilizable para reducir overhead.
- `int8_to_complexf(...)`
	- Convierte I/Q intercalado a complejo float.

### src/method3.c
Rol:
- Implementa el preproceso Metodo 3 sobre buffer en sitio.

Algoritmo:
1. Estima DC medio (I y Q).
2. Resta DC.
3. Calcula energia y RMS.
4. Normaliza amplitud por RMS.

Ventaja:
- O(n), sin asignaciones dinamicas.

Catalogo de funciones:
- `method3_dc_rms_norm(complexf_t *x, size_t n)`
	- Remueve DC en I/Q y normaliza por RMS global.

### src/welch.c
Rol:
- Implementa Welch PSD con FFT radix-2 y buffers reutilizables.

Responsabilidades:
- Inicializacion de workspace (ventana, bit-reversal, scratch).
- FFT in-place por segmento.
- Promedio espectral y fft-shift.
- Extraccion de metricas PSD.

Mejoras aplicadas:
- Precalculo de normalizacion para evitar divisiones repetidas por bin.
- Cache del eje de frecuencia cuando `fs` no cambia.
- Calculo del bin central por indice directo (`bins/2`) en espectro desplazado.
- Guardas de validacion para entradas nulas/invalidas.

Catalogo de funciones:
- `is_power_of_two(size_t n)`
	- Verifica condicion radix-2 para FFT.
- `fft_inplace(complexf_t *buf, size_t n, const size_t *bitrev)`
	- FFT iterativa in-place.
- `welch_workspace_init(welch_workspace_t *ws, int nperseg, float overlap, mem_tracker_t *mt)`
	- Prepara buffers, ventana Hann y indices de bit-reversal.
- `welch_workspace_free(welch_workspace_t *ws, mem_tracker_t *mt)`
	- Libera recursos del workspace.
- `welch_fft_shifted(...)`
	- Ejecuta Welch y retorna PSD/frecuencia desplazadas.
- `compute_metrics_from_psd(...)`
	- Calcula ruido, potencia central y SNR desde PSD lineal.

### src/profiler.c
Rol:
- Utilidades de tiempo, memoria de proceso y tracking interno de asignaciones.

Responsabilidades:
- Timer de alta resolucion.
- RSS actual y pico.
- Carga de RAM del sistema.
- Wrappers `mt_alloc` y `mt_free` con contadores.

Nota:
- El tracker interno complementa (no reemplaza) sanitizers.

Catalogo de funciones:
- `now_ms(void)`
	- Cronometro monotono de alta resolucion.
- `process_rss_mb(void)`
	- RSS actual del proceso en MB.
- `process_peak_rss_mb(void)`
	- Pico de RSS del proceso en MB.
- `system_ram_percent(void)`
	- Carga de RAM del sistema.
- `mem_tracker_init(mem_tracker_t *mt)`
	- Inicializa contador interno de asignaciones.
- `mt_alloc(mem_tracker_t *mt, size_t bytes)`
	- Reserva memoria y actualiza contadores.
- `mt_free(mem_tracker_t *mt, void *ptr, size_t bytes)`
	- Libera memoria y actualiza contadores.

## Evaluacion del workflow actual
Fortalezas:
- Pipeline claro y deterministico.
- Memoria acotada por diseno.
- Reutilizacion de buffers en ruta caliente.
- Salida estructurada para auditoria y regresion.

Cuellos de botella esperados:
- Welch/FFT domina el tiempo total.
- I/O y conversion tienen impacto menor en comparacion.

## Recomendaciones para proyecto real
1. Baseline reproducible:
- Fijar dataset, argumentos CLI y tipo de build (Release).
- Guardar resultados en JSON/CSV por commit.

2. Calidad de build:
- Mantener warnings estrictos y LTO en Release.
- Habilitar optimizacion nativa solo en benchmarks locales.

3. Rendimiento DSP:
- Evaluar backend FFT especializado (FFTW/KissFFT).
- Medir delta con A/B benchmark sobre mismo lote.

4. Confiabilidad:
- Agregar tests de regresion numerica para SNR/noise/center power.
- Integrar validacion de leaks en CI (ASan en Linux o herramientas equivalentes).

## Convenciones recomendadas para seguir documentando
1. Mantener encabezado de archivo con proposito y alcance.
2. Documentar cada funcion con: objetivo, entradas, salida y errores.
3. Registrar complejidad O(n) cuando aplique en funciones de procesamiento.
4. Mantener comentarios de optimizacion solo donde exista impacto real en rendimiento.

## Checklist operativo
- Build Release correcto.
- Sin fuga en `mem_tracker`.
- `welch_ms` medido y versionado.
- JSON/CSV generados en cada corrida.
- Comparacion de throughput y calidad sobre mismos parametros.
