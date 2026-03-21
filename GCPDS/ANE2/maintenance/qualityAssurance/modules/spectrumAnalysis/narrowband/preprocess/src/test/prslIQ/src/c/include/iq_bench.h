#ifndef IQ_BENCH_H
#define IQ_BENCH_H

/**
 * @file iq_bench.h
 * @brief Interfaz publica del benchmark C para procesamiento IQ Metodo 3.
 *
 * Flujo principal:
 * discovery -> metadata parse -> raw load -> int8 to complex -> method3 -> welch -> metrics
 */

#include <stddef.h>
#include <stdint.h>

#define MAX_PATH_LEN 512
#define MAX_PAIRS 128

typedef struct
{
    float re;
    float im;
} complexf_t;

typedef struct
{
    char data_path[MAX_PATH_LEN];
    char meta_path[MAX_PATH_LEN];
} sigmf_pair_t;

typedef struct
{
    uint32_t sample_rate;
    uint64_t center_freq;
} sigmf_meta_t;

typedef struct
{
    float noise_floor_dbm;
    float center_power_dbm;
    float snr_center_db;
} iq_metrics_t;

typedef struct
{
    double load_ms;
    double convert_ms;
    double preprocess_ms;
    double welch_ms;
    double metric_ms;
    double kde_ms;
    double total_ms;
} perf_timing_t;

typedef struct
{
    size_t alloc_count;
    size_t free_count;
    size_t bytes_current;
    size_t bytes_peak;
} mem_tracker_t;

/**
 * @brief Workspace reutilizable para Welch FFT y buffers auxiliares.
 */
typedef struct
{
    size_t nperseg;
    size_t step;
    float overlap;
    float window_power;
    float inv_norm;
    float *window;
    complexf_t *fft_buf;
    size_t *bitrev;
    float *psd_acc;
    float *freq_hz;
    float *psd_shifted;
    uint32_t cached_fs;
    int freq_ready;
} welch_workspace_t;

/** @name profiler.c */
/** @{ */
/** @brief Retorna tiempo monotono en milisegundos. */
double now_ms(void);
/** @brief Retorna RSS actual del proceso en MB. */
double process_rss_mb(void);
/** @brief Retorna RSS pico del proceso en MB. */
double process_peak_rss_mb(void);
/** @brief Retorna porcentaje de carga de RAM del sistema. */
double system_ram_percent(void);
/** @brief Inicializa el tracker de memoria interno. */
void mem_tracker_init(mem_tracker_t *mt);
/**
 * @brief Reserva memoria y actualiza contadores del tracker.
 * @param mt Tracker de memoria.
 * @param bytes Cantidad de bytes a reservar.
 * @return Puntero a memoria reservada o NULL si falla.
 */
void *mt_alloc(mem_tracker_t *mt, size_t bytes);
/**
 * @brief Libera memoria y actualiza contadores del tracker.
 * @param mt Tracker de memoria.
 * @param ptr Puntero previamente reservado.
 * @param bytes Tamano original de la reserva para contabilidad.
 */
void mt_free(mem_tracker_t *mt, void *ptr, size_t bytes);
/** @} */

/** @name fs_utils.c */
/** @{ */
/**
 * @brief Descubre pares SigMF data/meta dentro de un directorio.
 * @param db_dir Directorio base del dataset.
 * @param out_pairs Arreglo de salida con pares encontrados.
 * @param max_pairs Capacidad maxima del arreglo de salida.
 * @return Numero de pares validos encontrados.
 */
int discover_sigmf_pairs(const char *db_dir, sigmf_pair_t *out_pairs, int max_pairs);
/** @} */

/** @name sigmf_io.c */
/** @{ */
/**
 * @brief Parsea metadata SigMF y extrae frecuencia y sample rate.
 * @param meta_path Ruta al archivo .sigmf-meta.
 * @param out_meta Estructura de salida.
 * @return 1 en exito, 0 en error.
 */
int parse_sigmf_meta(const char *meta_path, sigmf_meta_t *out_meta);
/**
 * @brief Carga IQ int8 acotado por limite de muestras complejas.
 * @param data_path Ruta al archivo .sigmf-data.
 * @param out_raw Buffer de salida reservado dinamicamente.
 * @param out_bytes Cantidad de bytes realmente cargados.
 * @param max_complex_samples Maximo de muestras complejas a cargar.
 * @param mt Tracker de memoria para auditar reservas.
 * @return 1 en exito, 0 en error.
 */
int load_iq_int8_limited(const char *data_path, int8_t **out_raw, size_t *out_bytes, size_t max_complex_samples, mem_tracker_t *mt);
/**
 * @brief Carga IQ int8 por chunks dentro de un buffer preasignado.
 * @param data_path Ruta al archivo .sigmf-data.
 * @param raw_buf Buffer destino.
 * @param raw_capacity Capacidad total del buffer destino.
 * @param out_bytes Bytes efectivamente cargados.
 * @param chunk_bytes Tamano de lectura por bloque.
 * @return 1 en exito, 0 en error.
 */
int load_iq_int8_limited_chunked_into(const char *data_path, int8_t *raw_buf, size_t raw_capacity, size_t *out_bytes, size_t chunk_bytes);
/**
 * @brief Convierte IQ int8 intercalado a complejos float.
 * @param raw Buffer de entrada I,Q intercalado.
 * @param raw_bytes Tamano en bytes del buffer de entrada.
 * @param out_complex Arreglo de salida complejo.
 * @param out_n Capacidad maxima de salida en muestras complejas.
 */
void int8_to_complexf(const int8_t *raw, size_t raw_bytes, complexf_t *out_complex, size_t out_n);
/** @} */

/** @name method3.c */
/** @{ */
/**
 * @brief Aplica Metodo 3 (remocion DC + normalizacion RMS) en sitio.
 * @param x Arreglo complejo a procesar.
 * @param n Numero de muestras complejas.
 */
void method3_dc_rms_norm(complexf_t *x, size_t n);
/** @} */

/** @name welch.c */
/** @{ */
/**
 * @brief Inicializa workspace Welch y sus buffers reutilizables.
 * @param ws Workspace de salida.
 * @param nperseg Longitud de segmento FFT (potencia de 2).
 * @param overlap Solape entre segmentos [0,1).
 * @param mt Tracker de memoria para reservas.
 * @return 1 en exito, 0 en error.
 */
int welch_workspace_init(welch_workspace_t *ws, int nperseg, float overlap, mem_tracker_t *mt);
/**
 * @brief Libera recursos asociados a un workspace Welch.
 * @param ws Workspace a liberar.
 * @param mt Tracker de memoria.
 */
void welch_workspace_free(welch_workspace_t *ws, mem_tracker_t *mt);
/**
 * @brief Calcula Welch PSD y devuelve espectro/frecuencias desplazados.
 * @param x Senal compleja de entrada.
 * @param n Numero de muestras en entrada.
 * @param fs Frecuencia de muestreo en Hz.
 * @param ws Workspace Welch reutilizable.
 * @param out_freq_hz Salida: eje de frecuencias desplazado.
 * @param out_psd_linear Salida: PSD lineal desplazada.
 * @param out_bins Salida: numero de bins.
 * @return 1 en exito, 0 en error.
 */
int welch_fft_shifted(
    const complexf_t *x,
    size_t n,
    uint32_t fs,
    welch_workspace_t *ws,
    const float **out_freq_hz,
    const float **out_psd_linear,
    size_t *out_bins);

/**
 * @brief Calcula metricas de senal desde una PSD lineal.
 * @param freq_hz Eje de frecuencias.
 * @param psd_linear PSD lineal.
 * @param bins Cantidad de bins.
 * @param out_metrics Estructura de salida con ruido, centro y SNR.
 */
void compute_metrics_from_psd(
    const float *freq_hz,
    const float *psd_linear,
    size_t bins,
    iq_metrics_t *out_metrics);
/** @} */

#endif
