#include "iq_bench.h"

/*
 * welch.c
 * Reference Welch PSD implementation (naive DFT) and PSD-based metrics.
 */

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int welch_naive_shifted(
    const complexf_t *x,
    size_t n,
    uint32_t fs,
    int nperseg,
    float overlap,
    float **out_freq_hz,
    float **out_psd_linear,
    size_t *out_bins,
    mem_tracker_t *mt)
{
    /* Guardrails for invalid runtime configuration. */
    if (!x || n == 0 || nperseg < 8 || overlap < 0.0f || overlap >= 1.0f)
    {
        return 0;
    }

    size_t seg_len = (size_t)nperseg;
    if (n < seg_len)
    {
        return 0;
    }

    size_t step = (size_t)((1.0f - overlap) * (float)seg_len);
    if (step == 0)
    {
        step = 1;
    }

    size_t segments = 1 + (n - seg_len) / step;

    float *window = (float *)mt_alloc(mt, seg_len * sizeof(float));
    float *psd_acc = (float *)mt_alloc(mt, seg_len * sizeof(float));
    float *freq = (float *)mt_alloc(mt, seg_len * sizeof(float));
    float *psd_shifted = (float *)mt_alloc(mt, seg_len * sizeof(float));

    if (!window || !psd_acc || !freq || !psd_shifted)
    {
        if (window)
            mt_free(mt, window, seg_len * sizeof(float));
        if (psd_acc)
            mt_free(mt, psd_acc, seg_len * sizeof(float));
        if (freq)
            mt_free(mt, freq, seg_len * sizeof(float));
        if (psd_shifted)
            mt_free(mt, psd_shifted, seg_len * sizeof(float));
        return 0;
    }

    for (size_t i = 0; i < seg_len; i++)
    {
        window[i] = 0.5f - 0.5f * cosf((float)(2.0 * M_PI * (double)i / (double)(seg_len - 1)));
        psd_acc[i] = 0.0f;
    }

    float window_power = 0.0f;
    for (size_t i = 0; i < seg_len; i++)
    {
        window_power += window[i] * window[i];
    }

    /* Naive DFT Welch reference implementation.
       Replace with FFTWf/KissFFT for production speed. */
    for (size_t s = 0; s < segments; s++)
    {
        size_t start = s * step;

        for (size_t k = 0; k < seg_len; k++)
        {
            float re = 0.0f;
            float im = 0.0f;

            for (size_t t = 0; t < seg_len; t++)
            {
                size_t idx = start + t;
                float wr = x[idx].re * window[t];
                float wi = x[idx].im * window[t];

                float ang = (float)(-2.0 * M_PI * (double)k * (double)t / (double)seg_len);
                float ca = cosf(ang);
                float sa = sinf(ang);

                re += wr * ca - wi * sa;
                im += wr * sa + wi * ca;
            }

            float p = (re * re + im * im) / (window_power * (float)fs + 1e-20f);
            psd_acc[k] += p;
        }
    }

    for (size_t k = 0; k < seg_len; k++)
    {
        psd_acc[k] /= (float)segments;
    }

    size_t half = seg_len / 2U;
    for (size_t k = 0; k < seg_len; k++)
    {
        size_t src = (k + half) % seg_len;
        psd_shifted[k] = psd_acc[src];
        freq[k] = ((float)((int)k - (int)half) * (float)fs) / (float)seg_len;
    }

    mt_free(mt, psd_acc, seg_len * sizeof(float));
    mt_free(mt, window, seg_len * sizeof(float));

    *out_freq_hz = freq;
    *out_psd_linear = psd_shifted;
    *out_bins = seg_len;
    return 1;
}

void compute_metrics_from_psd(
    const float *freq_hz,
    const float *psd_linear,
    size_t bins,
    iq_metrics_t *out_metrics)
{
    /* Metrics computed in dB domain to match Python pipeline outputs. */
    float sum_db = 0.0f;
    float center_freq = 0.5f * (freq_hz[0] + freq_hz[bins - 1]);

    size_t idx_center = 0;
    float best_diff = fabsf(freq_hz[0] - center_freq);

    for (size_t i = 0; i < bins; i++)
    {
        float p_db = 10.0f * log10f(psd_linear[i] + 1e-12f);
        sum_db += p_db;

        float d = fabsf(freq_hz[i] - center_freq);
        if (d < best_diff)
        {
            best_diff = d;
            idx_center = i;
        }
    }

    float noise_floor_dbm = sum_db / (float)bins;
    float center_power_dbm = 10.0f * log10f(psd_linear[idx_center] + 1e-12f);

    out_metrics->noise_floor_dbm = noise_floor_dbm;
    out_metrics->center_power_dbm = center_power_dbm;
    out_metrics->snr_center_db = center_power_dbm - noise_floor_dbm;
}
