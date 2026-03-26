# FM Carrier estimation 

_objective_ 

# Notebook Spec FM Carrier v3 (Condensado)

## Alcance
Notebook de estimacion de frecuencia portadora FM para monitoreo SDR (HackRF), con validacion de cumplimiento regulatorio y resumen por nodo/canal.

## Parametros criticos
- Banda FM: 87.5 a 108.0 MHz.
- Resolucion aproximada: 1220 Hz/bin.
- Tolerancia dura: +-2 kHz.
- Umbral warning: +-1 kHz.
- Raster nominal: 100 kHz.

## Flujo funcional minimo
1. Carga de CSV por nodo.
2. Parseo de `pxx` y validacion de filas.
3. Promedio en escala lineal y conversion a dBm.
4. Correccion de reloj (PPM) con fallback.
5. Mascara DC en frecuencia central.
6. Deteccion de picos (`find_peaks`).
7. Refinamiento sub-bin (interpolacion cuadratica).
8. Calculo de metricas por portadora.
9. Clasificacion de compliance.
10. Agregacion por canal y por nodo.
11. Export y auditoria.

## Entradas y salidas
Entradas:
- CSVs con `pxx`, `start_freq_hz`, `end_freq_hz` y metadata opcional.

Salidas principales:
- `summary_df`: una fila por portadora detectada.
- `carrier_df`: agregado por canal nominal.
- `node_df`: agregado por nodo.
- `violations`: desvio > +-2 kHz.
- `warnings_df`: desvio entre +-1 y +-2 kHz.

## Metricas por portadora (core)
- Frecuencia estimada y nominal.
- Desviacion en Hz/kHz/PPM.
- SNR, prominence, ancho de pico.
- Confidence [0,1] + flags de calidad.
- Estado: COMPLIANT/WARNING/VIOLATION/UNVERIFIABLE.

## Regla de compliance
1. `|dev_hz| <= 1000`: COMPLIANT.
2. `1000 < |dev_hz| <= 2000`: WARNING.
3. `|dev_hz| > 2000`: VIOLATION.
4. Sin match de canal nominal: UNVERIFIABLE.

## Requisitos de implementacion para esta version
1. Configuracion central (dataclass) y validacion inicial.
2. Manejo de errores por tipo, no generico.
3. Reporte de calidad por etapa y razon.
4. Logging estructurado persistente.
5. Export reproducible en carpeta timestamped.

## Contratos tecnicos clave
- Sin NaN/Inf en arrays finales usados para deteccion.
- Sin operaciones logaritmicas inseguras.
- Sin propagacion silenciosa de fallos de carga.
- Sin resultados de compliance sin trazabilidad de config.

## Protocolo de ejecucion recomendado
1. Imprimir metadata de corrida: version, fecha, entorno, config.
2. Ejecutar pipeline por etapas con metricas.
3. Guardar resultados tabulares y reporte de auditoria.
4. Registrar resumen final de perdida de datos y compliance.

## Definition of Done (tecnico)
- Pipeline ejecuta sin fallos no controlados.
- Reporta perdida de datos por etapa y causa.
- Genera evidencias de auditoria reutilizables.
- Permite explicar resultados regulatorios de forma reproducible.


# error handling implementation 


## Error Handling Quickstart (Condensado)

## Objetivo
Definir un manejo de errores robusto para notebooks SDR/FM: evitar caidas, cuantificar perdida de datos y dejar evidencia auditable.

## Prioridad de implementacion (de obligatorio a deseable)
1. Estabilidad matematica.
2. Guard clauses.
3. Excepciones especificas.
4. Degradacion elegante.
5. Reporte de calidad de datos.
6. Logging estructurado.
7. Validacion sintetica/regresion.

## Minimo viable obligatorio
1. Log seguro:
- Usar `np.maximum(x, 1e-30)` antes de `log10`.
- Verificar `np.isfinite(...)` despues de operaciones clave.

2. Guard clauses tempranas:
- Archivo no legible, columnas faltantes, dataframe vacio, arreglos vacios.
- Si no hay datos validos: retornar vacio tipado (DataFrame o dict esperado), no romper flujo.

3. Excepciones especificas:
- Capturar: `FileNotFoundError`, `PermissionError`, `pd.errors.ParserError`, `UnicodeDecodeError`, `ValueError`.
- Errores desconocidos: `logger.critical(...); raise`.

## Patrones recomendados
```python
# 1) Log seguro
safe_db = 10.0 * np.log10(np.maximum(lin_power, 1e-30))

# 2) Guard clause
if df is None or df.empty or "pxx" not in df.columns:
    return None

# 3) Excepciones especificas
try:
    df = pd.read_csv(path)
except FileNotFoundError:
    logger.error(f"File not found: {path}")
    return None
except pd.errors.ParserError as e:
    logger.error(f"CSV parser error: {e}")
    return None
except Exception as e:
    logger.critical(f"Unexpected error: {type(e).__name__}: {e}")
    raise
```

## Degradacion elegante
Aplicar cascada de fallback en pasos no criticos:
1. Estimacion por referencia.
2. Override configurado.
3. Default conservador (ej. 0 PPM).

Siempre registrar bandera de calidad (confidence flags) para no ocultar incertidumbre.

## Reporte de calidad de datos (obligatorio en esta version)
Registrar por ejecucion:
- `files_attempted`, `files_loaded`, `files_failed`.
- `rows_parsed`, `rows_rejected`.
- `rejection_reasons` (Counter por causa).
- Metricas por etapa: Loading, Detection, Compliance, Export.

## Logging estructurado
- Nivel archivo: DEBUG.
- Nivel consola: INFO.
- Formato: timestamp, level, modulo, mensaje.
- Un log por corrida, con timestamp en nombre.

## Checklist de salida por corrida
- Carpeta timestamped con resultados.
- CSV de detecciones y resumenes.
- Reporte de auditoria con perdida por etapa/causa.
- Metadata: version, fecha/hora, config usada, entorno.

## Criterio de aceptacion
La corrida debe responder con evidencia:
1. Cuanto dato se perdio.
2. En que etapa.
3. Por que causa.
4. Bajo que configuracion.


# nb audited 

## FM Carrier Audit Checklist 

## Estado tecnico resumido
Fortalezas actuales:
1. Estabilidad matematica base correcta.
2. Guard clauses en carga/procesamiento.
3. Manejo razonable de nulos/vacios.

Brechas de mayor impacto:
1. Logging persistente insuficiente.
2. Data quality reporting incompleto.
3. Excepciones aun demasiado generales en puntos criticos.
4. Reproducibilidad parcial (metadata de corrida incompleta).

## Matriz prioridad-impacto-esfuerzo
1. Logging estructurado a archivo.
- Impacto: Alto.
- Esfuerzo: Bajo.
- Resultado: Trazabilidad y depuracion auditables.

2. Reporte de calidad por etapa.
- Impacto: Alto.
- Esfuerzo: Medio.
- Resultado: Visibilidad de perdida de datos por causa.

3. Excepciones especificas + re-raise desconocidas.
- Impacto: Medio-Alto.
- Esfuerzo: Bajo.
- Resultado: Menos bugs ocultos, diagnostico preciso.

4. Validaciones invariantes (NaN/Inf/rangos).
- Impacto: Medio.
- Esfuerzo: Bajo.
- Resultado: Menos resultados plausibles pero incorrectos.

5. Pruebas sinteticas basicas.
- Impacto: Medio.
- Esfuerzo: Medio.
- Resultado: Menos regresiones en cambios futuros.

## Checklist de implementacion (DoD-definition of done)
- [ ] Logger central con `FileHandler` y niveles.
- [ ] `DataQualityReport` con contadores y razones.
- [ ] Metricas por etapa: Loading/Detection/Compliance/Export.
- [ ] Reemplazo de `except Exception` por excepciones tipadas donde aplique.
- [ ] `critical + raise` para errores inesperados.
- [ ] Assertions o validaciones de invariantes en puntos clave.
- [ ] Export de resultados y auditoria en carpeta timestamped.
- [ ] Metadata de corrida impresa y persistida (version/config/fecha).

## Riesgos si no se implementa
1. Perdida silenciosa de datos.
2. Imposibilidad de reconstruir decisiones del pipeline.
3. Dificultad para justificar resultados regulatorios.
4. Alto tiempo de diagnostico en produccion.

## Secuencia recomendada
1. P0: excepciones + guard clauses + estabilidad matematica.
2. P1: logging estructurado + quality report.
3. P2: reproducibilidad y exportes estandar.
4. P3: testing sintetico y regresion.

## Cierre esperado
El sistema debe poder explicar de forma objetiva:
- Que entro, que salio y que se descarto.
- Por que se descarto.
- Que parametros gobernaron la corrida.
- Que nivel de confianza tienen los resultados.
