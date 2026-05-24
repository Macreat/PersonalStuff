# Benchmark C de Metodo 3 IQ

Implementacion C minimalista para evaluar procesamiento IQ (Metodo 3) con foco en rendimiento, estabilidad y calidad de metricas.

## Alcance
- IQ int8 intercalado -> complejo float32
- Preproceso Metodo 3: remocion de DC + normalizacion RMS
- Welch PSD con workspace reutilizable
- Metricas de senal: piso de ruido, potencia central, SNR
- Metricas computacionales: tiempos por etapa, throughput, CPU, RAM, RSS
- Exportacion opcional a JSON y CSV

## Estructura
- include/iq_bench.h: tipos y API publica
- src/main.c: flujo principal y reporte
- src/fs_utils.c: descubrimiento de pares SigMF
- src/sigmf_io.c: parseo de metadata y carga IQ
- src/method3.c: preproceso Metodo 3
- src/welch.c: Welch/FFT y metricas PSD
- src/profiler.c: tiempo y memoria
- docs/SOURCE_REFERENCE.md: documentacion tecnica por archivo

## Build
Desde el directorio del proyecto:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Optimizacion nativa local (solo benchmarking en la misma maquina):

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DIQBENCH_NATIVE_OPT=ON
cmake --build build --config Release
```

## Ejecucion
```powershell
.\build\Release\iq_method3_bench.exe --db "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz" --files 6 --max-complex 262144 --nperseg 512 --overlap 0.5 --chunk-bytes 65536 --json-out qa_after.json --csv-out qa_after.csv
```

Smoke test:

```powershell
.\build\Release\iq_method3_bench.exe --files 2 --max-complex 32768 --nperseg 256 --overlap 0.5
```

## Despliegue Remoto (Raspberry Pi 4)
Para desplegar este benchmark en una Raspberry Pi 4 Model B via SSH, consulte el flujo de trabajo detallado en:
- `../../SSH_WORKFLOW.md`

## Lectura rapida de resultados
- Si `welch_ms` domina, el cuello de botella esta en la etapa espectral.
- Si `alloc_count == free_count` y `bytes_current == 0`, no hay fuga en tracker interno.
- `throughput_sps`, `latency_ratio` y `file_jitter_ms` resumen salud operacional del pipeline.

## Recomendaciones para produccion
1. Comparar versiones con mismos parametros y mismo dataset.
2. Mantener build Release para benchmark oficial.
3. Versionar salidas JSON/CSV por corrida para detectar regresiones.
4. Priorizar optimizacion de Welch/FFT antes de microoptimizar I/O.
