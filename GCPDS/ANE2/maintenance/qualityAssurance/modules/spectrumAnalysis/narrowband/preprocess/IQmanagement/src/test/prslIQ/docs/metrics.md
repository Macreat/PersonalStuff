# Auditoria de metricas de procesamiento IQ

## Resumen ejecutivo
El mejor metodo en Python fue Metodo 3 IQ. En ambos pipelines (Python y C), la etapa Welch/PSD domina el costo total y es el principal cuello de botella.

 La comparacion correcta debe hacerse con metricas normalizadas: throughput, tiempo por muestra y calidad IQ bajo la misma configuracion.

## Datos reportados

### Python (Metodo 3 IQ)
- Archivos procesados: 6
- Muestras totales: 157,286,400
- Tiempo total: 39,544.82 ms
- Tiempo promedio por archivo: 6,590.80 ms
- Throughput: 3,977,420.76 muestras/s
- SNR promedio +- std: 3.0049 +- 0.0522 dB
- Piso de ruido promedio: -74.4821 dBm
- Potencia central promedio: -71.4771 dBm
- Delta RSS: -0.0352 MB

Distribucion de tiempo en Python:
- Load: 875.233 ms (2.21%)
- Convert: 2,481.176 ms (6.27%)
- Welch: 36,182.093 ms (91.50%)
- Metric: 0.014 ms (0.00%)
- KDE: 4.997 ms (0.01%)

Prioridad de optimizacion en Python:
1. Welch
2. Conversion
3. Carga
4. KDE
5. Metricas

### C (estado actual)
- Tiempo total reportado: ~50,309 ms para 6 archivos
- Welch: 99.86% del costo
- I/O, conversion y preprocesamiento: <0.1%
- Throughput reportado: ~31,263 muestras/s
- SNR promedio: 13 dB
- Piso de ruido promedio: -73 dBm
- Potencia central promedio: -60 dBm
- Delta RSS: 0.742 MB

## Hallazgos de auditoria

3. La conclusion tecnica principal es solida: Welch/PSD sigue siendo el cuello de botella.
4. Las metricas de calidad de senal no son equivalentes aun entre pipelines; falta alinear parametros de estimacion.

## Marco de salud de adquisicion (HackRF)
Para evaluar estabilidad y calidad del pipeline, se propone monitorear:

1. Eficiencia de throughput y tasa de perdida
2. Jitter por chunk y estabilidad temporal
3. Calidad IQ (DC offset, desbalance I/Q, error de cuadratura, clipping)
4. Memoria (uso actual y pico)
5. Uso de CPU
6. Puntaje compuesto de salud con estado: OK, Warning o Error

## Estrategia de optimizacion
1. FFT backend con FFTW y planes reutilizables
2. Evitar recreacion de planes por segmento
3. Preasignacion y reutilizacion de buffers y ventana Hann
4. Procesamiento en chunks grandes (streaming)
5. Registro de tiempos y metricas en JSON/CSV para detectar regresiones

Clave tecnica: FFT preplanificada + buffers reutilizables + gestion eficiente de chunks en la seccion I/O.

## Nota metodologica
Welch PSD se calcula en cuatro pasos:
1. Segmentar la senal (nperseg fijo)
2. Aplicar ventana (por ejemplo, Hann)
3. Calcular FFT por segmento y obtener potencia
4. Promediar espectros para estimar PSD estable

## Evidencia visual
![Metricas en C tras optimizacion FFT](CMetricsAfterFFTmanagement.png)

## Conclusiones
1. Welch PSD es el principal objetivo de optimizacion en ambos entornos.
2. El tiempo bruto no debe usarse para comparar eficiencia si cambia el numero de muestras.
3. La comparacion justa requiere throughput y costo por muestra bajo parametros equivalentes.
4. Las diferencias de SNR y potencia central sugieren configuraciones no alineadas, no necesariamente un error de implementacion.
5. El siguiente paso tecnico es homologar parametros de estimacion y volver a medir con el mismo dataset.
