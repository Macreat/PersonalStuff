#include "iq_bench.h"

/*
 * sigmf_io.c
 * Lightweight SigMF metadata extraction and bounded raw IQ loading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int extract_u64(const char *text, const char *key, uint64_t *out_value)
{
    /* Naive key search parser; sufficient for fixed SigMF metadata fields. */
    const char *p = strstr(text, key);
    if (!p)
    {
        return 0;
    }

    p += strlen(key);
    p = strchr(p, ':');
    if (!p)
    {
        return 0;
    }
    p++;

    while (*p == ' ' || *p == '\t')
    {
        p++;
    }

    char *endp = NULL;
    double v = strtod(p, &endp);
    if (endp == p)
    {
        return 0;
    }

    if (v < 0.0)
    {
        return 0;
    }

    *out_value = (uint64_t)(v + 0.5);
    return 1;
}

int parse_sigmf_meta(const char *meta_path, sigmf_meta_t *out_meta)
{
    /* Reads entire metadata file and extracts sample_rate and center_freq. */
    FILE *f = fopen(meta_path, "rb");
    if (!f)
    {
        return 0;
    }

    int ok = 0;
    char *buf = NULL;

    if (fseek(f, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    long sz = ftell(f);
    if (sz <= 0)
    {
        goto cleanup;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    buf = (char *)malloc((size_t)sz + 1);
    if (!buf)
    {
        goto cleanup;
    }

    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz)
    {
        goto cleanup;
    }
    buf[sz] = '\0';

    uint64_t sr = 0;
    uint64_t cf = 0;

    if (!extract_u64(buf, "core:sample_rate", &sr))
    {
        goto cleanup;
    }
    if (!extract_u64(buf, "core:frequency", &cf))
    {
        goto cleanup;
    }

    out_meta->sample_rate = (uint32_t)sr;
    out_meta->center_freq = cf;
    ok = 1;

cleanup:
    if (buf)
    {
        free(buf);
    }
    fclose(f);
    return ok;
}

int load_iq_int8_limited(const char *data_path, int8_t **out_raw, size_t *out_bytes, size_t max_complex_samples, mem_tracker_t *mt)
{
    /* Bounded file read to keep memory predictable for benchmarking. */
    FILE *f = fopen(data_path, "rb");
    if (!f)
    {
        return 0;
    }

    int ok = 0;
    int8_t *raw = NULL;

    if (fseek(f, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    long sz = ftell(f);
    if (sz <= 0)
    {
        goto cleanup;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    size_t max_bytes = max_complex_samples * 2U;
    size_t wanted = (size_t)sz;
    if (wanted > max_bytes)
    {
        wanted = max_bytes;
    }

    raw = (int8_t *)mt_alloc(mt, wanted);
    if (!raw)
    {
        goto cleanup;
    }

    size_t nread = fread(raw, 1, wanted, f);
    if (nread == 0)
    {
        goto cleanup;
    }

    *out_raw = raw;
    *out_bytes = nread;
    ok = 1;

cleanup:
    if (!ok && raw)
    {
        mt_free(mt, raw, wanted);
    }
    fclose(f);
    return ok;
}

int load_iq_int8_limited_chunked_into(const char *data_path, int8_t *raw_buf, size_t raw_capacity, size_t *out_bytes, size_t chunk_bytes)
{
    if (!data_path || !raw_buf || raw_capacity == 0 || !out_bytes)
    {
        return 0;
    }

    if (chunk_bytes == 0)
    {
        chunk_bytes = 64U * 1024U;
    }

    FILE *f = fopen(data_path, "rb");
    if (!f)
    {
        return 0;
    }

    int ok = 0;
    size_t total = 0;

    while (total < raw_capacity)
    {
        size_t remain = raw_capacity - total;
        size_t to_read = (remain < chunk_bytes) ? remain : chunk_bytes;
        size_t nread = fread(raw_buf + total, 1, to_read, f);

        total += nread;

        if (nread < to_read)
        {
            if (feof(f))
            {
                break;
            }
            if (ferror(f))
            {
                goto cleanup;
            }
        }
    }

    if (total == 0)
    {
        goto cleanup;
    }

    *out_bytes = total;
    ok = 1;

cleanup:
    fclose(f);
    return ok;
}

void int8_to_complexf(const int8_t *raw, size_t raw_bytes, complexf_t *out_complex, size_t out_n)
{
    /* Interleaved input format: I,Q,I,Q... -> complex float. */
    size_t n = raw_bytes / 2U;
    if (n > out_n)
    {
        n = out_n;
    }

    for (size_t i = 0; i < n; i++)
    {
        out_complex[i].re = (float)raw[2U * i];
        out_complex[i].im = (float)raw[2U * i + 1U];
    }
}
