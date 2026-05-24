#include "liquid_bench.h"
#include <math.h>
#include <string.h>

/**
 * @file welch_liquid.c
 * @brief Welch PSD implementation using Liquid-DSP FFT.
 */

int liquid_welch_workspace_init(liquid_welch_workspace_t *ws, int nperseg, float overlap, mem_tracker_t *mt)
{
    if (!ws || !mt || nperseg < 8 || overlap < 0.0f || overlap >= 1.0f) return 0;

    memset(ws, 0, sizeof(*ws));
    ws->nperseg = (size_t)nperseg;
    ws->overlap = overlap;
    ws->step = (size_t)((1.0f - overlap) * (float)ws->nperseg);
    if (ws->step == 0) ws->step = 1;

    ws->window = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->fft_buf = (liquid_complexf_t *)mt_alloc(mt, ws->nperseg * sizeof(liquid_complexf_t));
    ws->psd_acc = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->freq_hz = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->psd_shifted = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));

    if (!ws->window || !ws->fft_buf || !ws->psd_acc || !ws->freq_hz || !ws->psd_shifted) {
        liquid_welch_workspace_free(ws, mt);
        return 0;
    }

    // Initialize Hann window using Liquid-DSP (or custom)
    for (size_t i = 0; i < ws->nperseg; i++) {
        ws->window[i] = 0.5f - 0.5f * cosf(2.0f * M_PI * i / (float)(ws->nperseg - 1));
    }

    // Create Liquid-DSP FFT plan
    ws->q = fft_create_plan(ws->nperseg, ws->fft_buf, ws->fft_buf, LIQUID_FFT_FORWARD, 0);

    ws->cached_fs = 0;
    ws->freq_ready = 0;

    return 1;
}

void liquid_welch_workspace_free(liquid_welch_workspace_t *ws, mem_tracker_t *mt)
{
    if (!ws || !mt) return;

    if (ws->q) fft_destroy_plan(ws->q);
    if (ws->psd_shifted) mt_free(mt, ws->psd_shifted, ws->nperseg * sizeof(float));
    if (ws->freq_hz) mt_free(mt, ws->freq_hz, ws->nperseg * sizeof(float));
    if (ws->psd_acc) mt_free(mt, ws->psd_acc, ws->nperseg * sizeof(float));
    if (ws->fft_buf) mt_free(mt, ws->fft_buf, ws->nperseg * sizeof(liquid_complexf_t));
    if (ws->window) mt_free(mt, ws->window, ws->nperseg * sizeof(float));

    memset(ws, 0, sizeof(*ws));
}

int liquid_welch_fft_shifted(
    const liquid_complexf_t *x,
    size_t n,
    uint32_t fs,
    liquid_welch_workspace_t *ws,
    const float **out_freq_hz,
    const float **out_psd_linear,
    size_t *out_bins)
{
    if (!x || !ws || !out_freq_hz || !out_psd_linear || !out_bins) return 0;
    if (n < ws->nperseg) return 0;

    size_t segments = 1 + (n - ws->nperseg) / ws->step;
    memset(ws->psd_acc, 0, ws->nperseg * sizeof(float));

    for (size_t s = 0; s < segments; s++) {
        size_t start = s * ws->step;

        for (size_t t = 0; t < ws->nperseg; t++) {
            ws->fft_buf[t] = x[start + t] * ws->window[t];
        }

        fft_execute(ws->q);

        for (size_t k = 0; k < ws->nperseg; k++) {
            float p = crealf(ws->fft_buf[k]) * crealf(ws->fft_buf[k]) + cimagf(ws->fft_buf[k]) * cimagf(ws->fft_buf[k]);
            ws->psd_acc[k] += p;
        }
    }

    // Normalization and shifting
    float norm = 1.0f / (float)segments;
    size_t half = ws->nperseg / 2;

    for (size_t k = 0; k < ws->nperseg; k++) {
        size_t shifted_idx = (k + half) % ws->nperseg;
        ws->psd_shifted[k] = ws->psd_acc[shifted_idx] * norm;
    }

    if (!ws->freq_ready || ws->cached_fs != fs) {
        for (size_t k = 0; k < ws->nperseg; k++) {
            ws->freq_hz[k] = ((float)((int)k - (int)half) * (float)fs) / (float)ws->nperseg;
        }
        ws->cached_fs = fs;
        ws->freq_ready = 1;
    }

    *out_freq_hz = ws->freq_hz;
    *out_psd_linear = ws->psd_shifted;
    *out_bins = ws->nperseg;

    return 1;
}

void liquid_compute_metrics_from_psd(
    const float *freq_hz,
    const float *psd_linear,
    size_t bins,
    iq_metrics_t *out_metrics)
{
    if (!freq_hz || !psd_linear || !out_metrics || bins == 0) return;

    float sum_db = 0.0f;
    size_t idx_center = bins / 2;

    for (size_t i = 0; i < bins; i++) {
        float p_db = 10.0f * log10f(psd_linear[i] + 1e-12f);
        sum_db += p_db;
    }

    float noise_floor_dbm = sum_db / (float)bins;
    float center_power_dbm = 10.0f * log10f(psd_linear[idx_center] + 1e-12f);

    out_metrics->noise_floor_dbm = noise_floor_dbm;
    out_metrics->center_power_dbm = center_power_dbm;
    out_metrics->snr_center_db = center_power_dbm - noise_floor_dbm;
}
