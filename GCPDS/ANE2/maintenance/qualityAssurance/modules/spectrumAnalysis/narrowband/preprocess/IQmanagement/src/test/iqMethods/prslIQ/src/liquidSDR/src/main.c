#include "liquid_bench.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @file main.c
 * @brief CLI entry point for Liquid-DSP IQ Method 3 benchmark.
 */

static void print_usage(const char *exe)
{
    printf("Usage:\n");
    printf("  %s [--db <path>] [--files <N>] [--max-complex <N>] [--nperseg <N>] [--overlap <0..0.9>] [--chunk-bytes <N>] [--json-out <file>] [--csv-out <file>]\n", exe);
}

static int write_json_summary(const char *path, const perf_timing_t *t, double total_samples, double throughput, double rss_peak, int files_ok, int files_req)
{
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "{\n");
    fprintf(f, "  \"files_requested\": %d,\n", files_req);
    fprintf(f, "  \"files_processed\": %d,\n", files_ok);
    fprintf(f, "  \"samples_total\": %.0f,\n", total_samples);
    fprintf(f, "  \"throughput_sps\": %.6f,\n", throughput);
    fprintf(f, "  \"rss_peak_mb\": %.6f,\n", rss_peak);
    fprintf(f, "  \"total_ms\": %.6f,\n", t->total_ms);
    fprintf(f, "  \"load_ms\": %.6f,\n", t->load_ms);
    fprintf(f, "  \"convert_ms\": %.6f,\n", t->convert_ms);
    fprintf(f, "  \"preprocess_ms\": %.6f,\n", t->preprocess_ms);
    fprintf(f, "  \"welch_ms\": %.6f,\n", t->welch_ms);
    fprintf(f, "  \"metric_ms\": %.6f\n", t->metric_ms);
    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

int main(int argc, char **argv)
{
    const char *db_dir = "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz";
    int files_to_use = 6;
    size_t max_complex = 262144;
    int nperseg = 512;
    float overlap = 0.5f;
    size_t chunk_bytes = 65536;
    const char *json_out = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--db") == 0 && i + 1 < argc) db_dir = argv[++i];
        else if (strcmp(argv[i], "--files") == 0 && i + 1 < argc) files_to_use = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-complex") == 0 && i + 1 < argc) max_complex = (size_t)strtoull(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--nperseg") == 0 && i + 1 < argc) nperseg = atoi(argv[++i]);
        else if (strcmp(argv[i], "--overlap") == 0 && i + 1 < argc) overlap = (float)atof(argv[++i]);
        else if (strcmp(argv[i], "--chunk-bytes") == 0 && i + 1 < argc) chunk_bytes = (size_t)strtoull(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--json-out") == 0 && i + 1 < argc) json_out = argv[++i];
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) { print_usage(argv[0]); return 0; }
    }

    sigmf_pair_t pairs[MAX_PAIRS];
    int total_pairs = discover_sigmf_pairs(db_dir, pairs, MAX_PAIRS);
    if (total_pairs <= 0) { printf("No SigMF pairs found\n"); return 1; }
    if (files_to_use > total_pairs) files_to_use = total_pairs;

    mem_tracker_t mt;
    mem_tracker_init(&mt);

    int8_t *raw_buf = (int8_t *)mt_alloc(&mt, max_complex * 2);
    liquid_complexf_t *iq_buf = (liquid_complexf_t *)mt_alloc(&mt, max_complex * sizeof(liquid_complexf_t));
    
    liquid_welch_workspace_t ws;
    if (!liquid_welch_workspace_init(&ws, nperseg, overlap, &mt)) { printf("Welch init failed\n"); return 1; }

    perf_timing_t t = {0};
    double total_start = now_ms();
    double total_samples = 0;

    printf("================================================================================\n");
    printf("LIQUID-DSP HYBRID BENCHMARK (METHOD 3)\n");
    printf("================================================================================\n");

    int files_processed = 0;
    for (int i = 0; i < files_to_use; i++)
    {
        sigmf_meta_t meta = {0};
        size_t raw_bytes = 0;
        const float *freq = NULL;
        const float *psd = NULL;
        size_t bins = 0;
        iq_metrics_t m = {0};

        if (!parse_sigmf_meta(pairs[i].meta_path, &meta)) continue;

        double s = now_ms();
        if (!load_iq_int8_limited_chunked_into(pairs[i].data_path, raw_buf, max_complex * 2, &raw_bytes, chunk_bytes)) continue;
        t.load_ms += now_ms() - s;

        size_t n_complex = raw_bytes / 2;
        total_samples += (double)n_complex;

        s = now_ms();
        int8_to_liquid_complexf(raw_buf, raw_bytes, iq_buf, n_complex);
        t.convert_ms += now_ms() - s;

        s = now_ms();
        liquid_method3_dc_rms_norm(iq_buf, n_complex);
        t.preprocess_ms += now_ms() - s;

        s = now_ms();
        if (!liquid_welch_fft_shifted(iq_buf, n_complex, meta.sample_rate, &ws, &freq, &psd, &bins)) continue;
        t.welch_ms += now_ms() - s;

        s = now_ms();
        liquid_compute_metrics_from_psd(freq, psd, bins, &m);
        t.metric_ms += now_ms() - s;

        files_processed++;
        printf("%02d) SNR=%7.3f dB | NF=%8.3f dBm | CP=%8.3f dBm\n", i + 1, m.snr_center_db, m.noise_floor_dbm, m.center_power_dbm);
    }

    t.total_ms = now_ms() - total_start;
    double throughput = (t.total_ms > 0) ? (total_samples / (t.total_ms / 1000.0)) : 0;
    double rss_peak = process_peak_rss_mb();

    printf("\nSUMMARY (Liquid-DSP Hybrid)\n");
    printf("total_ms       : %.3f\n", t.total_ms);
    printf("throughput_sps : %.2f\n", throughput);
    printf("rss_peak_mb    : %.3f\n", rss_peak);

    if (json_out) write_json_summary(json_out, &t, total_samples, throughput, rss_peak, files_processed, files_to_use);

    liquid_welch_workspace_free(&ws, &mt);
    mt_free(&mt, iq_buf, max_complex * sizeof(liquid_complexf_t));
    mt_free(&mt, raw_buf, max_complex * 2);

    return 0;
}
