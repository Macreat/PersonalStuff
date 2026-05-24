#ifndef LIQUID_BENCH_H
#define LIQUID_BENCH_H

/**
 * @file liquid_bench.h
 * @brief Public interface for the Liquid-DSP benchmark for IQ Method 3.
 */

#include <stddef.h>
#include <stdint.h>
#include <complex.h>
#include <liquid/liquid.h>

#define MAX_PATH_LEN 512
#define MAX_PAIRS 128

/* Use standard C99 complex for Liquid-DSP compatibility */
typedef float complex liquid_complexf_t;

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
 * @brief Reusable workspace for Liquid-DSP Welch PSD.
 */
typedef struct
{
    size_t nperseg;
    size_t step;
    float overlap;
    
    float *window;
    liquid_complexf_t *fft_buf;
    float *psd_acc;
    float *freq_hz;
    float *psd_shifted;
    
    fftplan q; // Liquid-DSP FFT plan
    
    uint32_t cached_fs;
    int freq_ready;
} liquid_welch_workspace_t;

/** @name profiler.c */
double now_ms(void);
double process_rss_mb(void);
double process_peak_rss_mb(void);
double system_ram_percent(void);
void mem_tracker_init(mem_tracker_t *mt);
void *mt_alloc(mem_tracker_t *mt, size_t bytes);
void mt_free(mem_tracker_t *mt, void *ptr, size_t bytes);

/** @name fs_utils.c */
int discover_sigmf_pairs(const char *db_dir, sigmf_pair_t *out_pairs, int max_pairs);

/** @name sigmf_io_liquid.c */
int parse_sigmf_meta(const char *meta_path, sigmf_meta_t *out_meta);
int load_iq_int8_limited_chunked_into(const char *data_path, int8_t *raw_buf, size_t raw_capacity, size_t *out_bytes, size_t chunk_bytes);
void int8_to_liquid_complexf(const int8_t *raw, size_t raw_bytes, liquid_complexf_t *out_complex, size_t out_n);

/** @name liquid_method3.c */
void liquid_method3_dc_rms_norm(liquid_complexf_t *x, size_t n);

/** @name welch_liquid.c (Part of main or separate) */
int liquid_welch_workspace_init(liquid_welch_workspace_t *ws, int nperseg, float overlap, mem_tracker_t *mt);
void liquid_welch_workspace_free(liquid_welch_workspace_t *ws, mem_tracker_t *mt);
int liquid_welch_fft_shifted(
    const liquid_complexf_t *x,
    size_t n,
    uint32_t fs,
    liquid_welch_workspace_t *ws,
    const float **out_freq_hz,
    const float **out_psd_linear,
    size_t *out_bins);

void liquid_compute_metrics_from_psd(
    const float *freq_hz,
    const float *psd_linear,
    size_t bins,
    iq_metrics_t *out_metrics);

#endif
