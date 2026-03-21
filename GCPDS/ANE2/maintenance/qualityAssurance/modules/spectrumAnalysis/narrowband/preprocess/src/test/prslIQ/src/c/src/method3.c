#include "iq_bench.h"

/*
 * method3.c
 * Method 3 preprocessing: DC removal + RMS normalization in-place.
 */

#include <math.h>

/*
 * method3_dc_rms_norm
 * Aplica Metodo 3 en sitio:
 * 1) estimacion/remocion DC en I y Q
 * 2) normalizacion RMS global
 * Complejidad temporal: O(n). Memoria extra: O(1).
 */
void method3_dc_rms_norm(complexf_t *x, size_t n)
{
    if (!x || n == 0)
    {
        return;
    }

    float mean_re = 0.0f;
    float mean_im = 0.0f;

    for (size_t i = 0; i < n; i++)
    {
        /* First pass: estimate DC component. */
        mean_re += x[i].re;
        mean_im += x[i].im;
    }

    mean_re /= (float)n;
    mean_im /= (float)n;

    float energy = 0.0f;
    for (size_t i = 0; i < n; i++)
    {
        /* Second pass: remove DC and accumulate energy for RMS. */
        x[i].re -= mean_re;
        x[i].im -= mean_im;
        energy += x[i].re * x[i].re + x[i].im * x[i].im;
    }

    float rms = sqrtf(energy / (float)n);
    if (rms > 1e-12f)
    {
        /* Final pass: normalize to unit RMS to stabilize PSD scale. */
        float inv_rms = 1.0f / rms;
        for (size_t i = 0; i < n; i++)
        {
            x[i].re *= inv_rms;
            x[i].im *= inv_rms;
        }
    }
}
