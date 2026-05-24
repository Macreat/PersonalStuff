#include "liquid_bench.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file sigmf_io_liquid.c
 * @brief SigMF metadata parsing and binary I/O for Liquid-DSP.
 */

int parse_sigmf_meta(const char *meta_path, sigmf_meta_t *out_meta)
{
    if (!meta_path || !out_meta) return 0;

    FILE *f = fopen(meta_path, "r");
    if (!f) return 0;

    char line[1024];
    int found_fs = 0;
    int found_cf = 0;

    // Minimal JSON-ish parser for core fields
    while (fgets(line, sizeof(line), f))
    {
        if (strstr(line, "sample_rate"))
        {
            char *p = strchr(line, ':');
            if (p) {
                out_meta->sample_rate = (uint32_t)strtoul(p + 1, NULL, 10);
                found_fs = 1;
            }
        }
        if (strstr(line, "center_frequency"))
        {
            char *p = strchr(line, ':');
            if (p) {
                out_meta->center_freq = (uint64_t)strtoull(p + 1, NULL, 10);
                found_cf = 1;
            }
        }
    }

    fclose(f);
    return found_fs; // At least sample rate is required
}

int load_iq_int8_limited_chunked_into(const char *data_path, int8_t *raw_buf, size_t raw_capacity, size_t *out_bytes, size_t chunk_bytes)
{
    if (!data_path || !raw_buf) return 0;

    FILE *f = fopen(data_path, "rb");
    if (!f) return 0;

    size_t total_read = 0;
    size_t bytes_to_read = raw_capacity;

    while (total_read < bytes_to_read)
    {
        size_t to_read = chunk_bytes;
        if (total_read + to_read > bytes_to_read) {
            to_read = bytes_to_read - total_read;
        }

        size_t n = fread(raw_buf + total_read, 1, to_read, f);
        if (n == 0) break;
        total_read += n;
    }

    *out_bytes = total_read;
    fclose(f);
    return 1;
}

void int8_to_liquid_complexf(const int8_t *raw, size_t raw_bytes, liquid_complexf_t *out_complex, size_t out_n)
{
    if (!raw || !out_complex) return;

    size_t samples = raw_bytes / 2;
    if (samples > out_n) samples = out_n;

    const float scale = 1.0f / 128.0f;

    for (size_t i = 0; i < samples; i++)
    {
        float re = (float)raw[2 * i] * scale;
        float im = (float)raw[2 * i + 1] * scale;
        out_complex[i] = re + im * I;
    }
}
