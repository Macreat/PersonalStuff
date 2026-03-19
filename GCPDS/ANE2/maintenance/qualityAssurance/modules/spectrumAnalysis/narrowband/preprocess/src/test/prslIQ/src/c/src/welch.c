#include "iq_bench.h"

/*
 * welch.c
 * FFT-based Welch PSD using reusable workspace buffers.
 */

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int is_power_of_two(size_t n)
{
    return (n != 0) && ((n & (n - 1U)) == 0U);
}

static void fft_inplace(complexf_t *buf, size_t n, const size_t *bitrev)
{
    for (size_t i = 0; i < n; i++)
    {
        size_t j = bitrev[i];
        if (j > i)
        {
            complexf_t tmp = buf[i];
            buf[i] = buf[j];
            buf[j] = tmp;
        }
    }

    for (size_t len = 2; len <= n; len <<= 1U)
    {
        float ang = (float)(-2.0 * M_PI / (double)len);
        float wlen_re = cosf(ang);
        float wlen_im = sinf(ang);

        for (size_t i = 0; i < n; i += len)
        {
            float w_re = 1.0f;
            float w_im = 0.0f;
            size_t half = len >> 1U;

            for (size_t j = 0; j < half; j++)
            {
                complexf_t u = buf[i + j];
                complexf_t v = buf[i + j + half];

                float vr = v.re * w_re - v.im * w_im;
                float vi = v.re * w_im + v.im * w_re;

                buf[i + j].re = u.re + vr;
                buf[i + j].im = u.im + vi;
                buf[i + j + half].re = u.re - vr;
                buf[i + j + half].im = u.im - vi;

                float nw_re = w_re * wlen_re - w_im * wlen_im;
                float nw_im = w_re * wlen_im + w_im * wlen_re;
                w_re = nw_re;
                w_im = nw_im;
            }
        }
    }
}

int welch_workspace_init(welch_workspace_t *ws, int nperseg, float overlap, mem_tracker_t *mt)
{
    if (!ws || !mt || nperseg < 8 || overlap < 0.0f || overlap >= 1.0f)
    {
        return 0;
    }

    memset(ws, 0, sizeof(*ws));
    ws->nperseg = (size_t)nperseg;
    ws->overlap = overlap;
    ws->step = (size_t)((1.0f - overlap) * (float)ws->nperseg);
    if (ws->step == 0)
    {
        ws->step = 1;
    }

    if (!is_power_of_two(ws->nperseg))
    {
        return 0;
    }

    ws->window = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->fft_buf = (complexf_t *)mt_alloc(mt, ws->nperseg * sizeof(complexf_t));
    ws->bitrev = (size_t *)mt_alloc(mt, ws->nperseg * sizeof(size_t));
    ws->psd_acc = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->freq_hz = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));
    ws->psd_shifted = (float *)mt_alloc(mt, ws->nperseg * sizeof(float));

    if (!ws->window || !ws->fft_buf || !ws->bitrev || !ws->psd_acc || !ws->freq_hz || !ws->psd_shifted)
    {
        welch_workspace_free(ws, mt);
        return 0;
    }

    ws->window_power = 0.0f;
    for (size_t i = 0; i < ws->nperseg; i++)
    {
        ws->window[i] = 0.5f - 0.5f * cosf((float)(2.0 * M_PI * (double)i / (double)(ws->nperseg - 1U)));
        ws->window_power += ws->window[i] * ws->window[i];
    }

    size_t bits = 0;
    for (size_t t = ws->nperseg; t > 1U; t >>= 1U)
    {
        bits++;
    }

    for (size_t i = 0; i < ws->nperseg; i++)
    {
        size_t x = i;
        size_t r = 0;
        for (size_t b = 0; b < bits; b++)
        {
            r = (r << 1U) | (x & 1U);
            x >>= 1U;
        }
        ws->bitrev[i] = r;
    }

    return 1;
}

void welch_workspace_free(welch_workspace_t *ws, mem_tracker_t *mt)
{
    if (!ws || !mt)
    {
        return;
    }

    if (ws->psd_shifted)
    {
        mt_free(mt, ws->psd_shifted, ws->nperseg * sizeof(float));
    }
    if (ws->freq_hz)
    {
        mt_free(mt, ws->freq_hz, ws->nperseg * sizeof(float));
    }
    if (ws->psd_acc)
    {
        mt_free(mt, ws->psd_acc, ws->nperseg * sizeof(float));
    }
    if (ws->bitrev)
    {
        mt_free(mt, ws->bitrev, ws->nperseg * sizeof(size_t));
    }
    if (ws->fft_buf)
    {
        mt_free(mt, ws->fft_buf, ws->nperseg * sizeof(complexf_t));
    }
    if (ws->window)
    {
        mt_free(mt, ws->window, ws->nperseg * sizeof(float));
    }

    memset(ws, 0, sizeof(*ws));
}

int welch_fft_shifted(
    const complexf_t *x,
    size_t n,
    uint32_t fs,
    welch_workspace_t *ws,
    const float **out_freq_hz,
    const float **out_psd_linear,
    size_t *out_bins)
{
    if (!x || !ws || !out_freq_hz || !out_psd_linear || !out_bins)
    {
        return 0;
    }

    if (n < ws->nperseg)
    {
        return 0;
    }

    size_t segments = 1U + (n - ws->nperseg) / ws->step;
    memset(ws->psd_acc, 0, ws->nperseg * sizeof(float));

    for (size_t s = 0; s < segments; s++)
    {
        size_t start = s * ws->step;

        for (size_t t = 0; t < ws->nperseg; t++)
        {
            size_t idx = start + t;
            ws->fft_buf[t].re = x[idx].re * ws->window[t];
            ws->fft_buf[t].im = x[idx].im * ws->window[t];
        }

        fft_inplace(ws->fft_buf, ws->nperseg, ws->bitrev);

        for (size_t k = 0; k < ws->nperseg; k++)
        {
            float re = ws->fft_buf[k].re;
            float im = ws->fft_buf[k].im;
            float p = (re * re + im * im) / (ws->window_power * (float)fs + 1e-20f);
            ws->psd_acc[k] += p;
        }
    }

    for (size_t k = 0; k < ws->nperseg; k++)
    {
        ws->psd_acc[k] /= (float)segments;
    }

    size_t half = ws->nperseg / 2U;
    for (size_t k = 0; k < ws->nperseg; k++)
    {
        size_t src = (k + half) % ws->nperseg;
        ws->psd_shifted[k] = ws->psd_acc[src];
        ws->freq_hz[k] = ((float)((int)k - (int)half) * (float)fs) / (float)ws->nperseg;
    }

    *out_freq_hz = ws->freq_hz;
    *out_psd_linear = ws->psd_shifted;
    *out_bins = ws->nperseg;
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
