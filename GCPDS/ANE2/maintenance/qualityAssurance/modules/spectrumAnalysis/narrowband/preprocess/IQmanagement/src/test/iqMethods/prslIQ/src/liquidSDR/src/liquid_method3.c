#include "liquid_bench.h"
#include <math.h>

/**
 * @file liquid_method3.c
 * @brief IQ Calibration Method 3 using Liquid-DSP primitives.
 */

void liquid_method3_dc_rms_norm(liquid_complexf_t *x, size_t n)
{
    if (!x || n == 0) return;

    liquid_complexf_t sum = 0;
    
    // Pass 1: Estimate DC
    for (size_t i = 0; i < n; i++) {
        sum += x[i];
    }
    
    liquid_complexf_t mean = sum / (float)n;
    
    float energy = 0.0f;
    
    // Pass 2: Remove DC and calculate energy
    for (size_t i = 0; i < n; i++) {
        x[i] -= mean;
        energy += crealf(x[i]) * crealf(x[i]) + cimagf(x[i]) * cimagf(x[i]);
    }
    
    float rms = sqrtf(energy / (float)n);
    
    if (rms > 1e-12f) {
        float inv_rms = 1.0f / rms;
        // Pass 3: Normalize
        for (size_t i = 0; i < n; i++) {
            x[i] *= inv_rms;
        }
    }
}
