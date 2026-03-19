#include "iq_bench.h"

/*
 * main.c
 * CLI entrypoint for Method 3 benchmark and audit reporting.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *exe)
{
    /* Keep options minimal and reproducible for benchmark comparisons. */
    printf("Usage:\n");
    printf("  %s [--db <path>] [--files <N>] [--max-complex <N>] [--nperseg <N>] [--overlap <0..0.9>]\n", exe);
    printf("\nDefaults:\n");
    printf("  --db D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz\n");
    printf("  --files 6\n");
    printf("  --max-complex 262144\n");
    printf("  --nperseg 512\n");
    printf("  --overlap 0.5\n");
}

int main(int argc, char **argv)
{
    /* Defaults tuned for quick local runs; override via CLI. */
    const char *db_dir = "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz";
    int files_to_use = 6;
    size_t max_complex = 262144;
    int nperseg = 512;
    float overlap = 0.5f;

    for (int i = 1; i < argc; i++)
    {
        /* Simple argument parser, fail-fast on unknown options. */
        if (strcmp(argv[i], "--db") == 0 && i + 1 < argc)
        {
            db_dir = argv[++i];
        }
        else if (strcmp(argv[i], "--files") == 0 && i + 1 < argc)
        {
            files_to_use = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--max-complex") == 0 && i + 1 < argc)
        {
            max_complex = (size_t)strtoull(argv[++i], NULL, 10);
        }
        else if (strcmp(argv[i], "--nperseg") == 0 && i + 1 < argc)
        {
            nperseg = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--overlap") == 0 && i + 1 < argc)
        {
            overlap = (float)atof(argv[++i]);
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        else
        {
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    sigmf_pair_t pairs[MAX_PAIRS];
    int total_pairs = discover_sigmf_pairs(db_dir, pairs, MAX_PAIRS);

    if (total_pairs <= 0)
    {
        printf("No SigMF pairs found in: %s\n", db_dir);
        return 1;
    }

    if (files_to_use > total_pairs)
    {
        files_to_use = total_pairs;
    }

    mem_tracker_t mt;
    mem_tracker_init(&mt);

    perf_timing_t t = {0};
    double total_start = now_ms();
    double rss_start = process_rss_mb();

    double sum_noise = 0.0;
    double sum_center = 0.0;
    double sum_snr = 0.0;
    double sum_snr2 = 0.0;
    double total_samples = 0.0;

    printf("================================================================================\n");
    printf("METHOD 3 C BENCHMARK (DC + RMS NORM)\n");
    printf("================================================================================\n");
    printf("DB: %s\n", db_dir);
    printf("Files: %d / %d\n", files_to_use, total_pairs);
    printf("max_complex=%llu nperseg=%d overlap=%.2f\n\n", (unsigned long long)max_complex, nperseg, overlap);

    for (int i = 0; i < files_to_use; i++)
    {
        /* Per-file ownership: allocate, process, free in the same scope. */
        sigmf_meta_t meta = {0};
        int8_t *raw = NULL;
        size_t raw_bytes = 0;
        complexf_t *iq = NULL;
        float *freq = NULL;
        float *psd = NULL;
        size_t bins = 0;
        iq_metrics_t m = {0};

        double file_start = now_ms();

        if (!parse_sigmf_meta(pairs[i].meta_path, &meta))
        {
            printf("[WARN] Meta parse failed: %s\n", pairs[i].meta_path);
            continue;
        }

        double s = now_ms();
        if (!load_iq_int8_limited(pairs[i].data_path, &raw, &raw_bytes, max_complex, &mt))
        {
            printf("[WARN] Data load failed: %s\n", pairs[i].data_path);
            continue;
        }
        t.load_ms += now_ms() - s;

        size_t n_complex = raw_bytes / 2U;
        total_samples += (double)n_complex;

        s = now_ms();
        iq = (complexf_t *)mt_alloc(&mt, n_complex * sizeof(complexf_t));
        if (!iq)
        {
            printf("[ERR] Out of memory for IQ buffer\n");
            mt_free(&mt, raw, raw_bytes);
            return 2;
        }
        int8_to_complexf(raw, raw_bytes, iq, n_complex);
        t.convert_ms += now_ms() - s;

        s = now_ms();
        method3_dc_rms_norm(iq, n_complex);
        t.preprocess_ms += now_ms() - s;

        s = now_ms();
        if (!welch_naive_shifted(iq, n_complex, meta.sample_rate, nperseg, overlap, &freq, &psd, &bins, &mt))
        {
            printf("[WARN] Welch failed for: %s\n", pairs[i].data_path);
            mt_free(&mt, iq, n_complex * sizeof(complexf_t));
            mt_free(&mt, raw, raw_bytes);
            continue;
        }
        t.welch_ms += now_ms() - s;

        s = now_ms();
        compute_metrics_from_psd(freq, psd, bins, &m);
        t.metric_ms += now_ms() - s;

        sum_noise += m.noise_floor_dbm;
        sum_center += m.center_power_dbm;
        sum_snr += m.snr_center_db;
        sum_snr2 += m.snr_center_db * m.snr_center_db;

        printf("%02d) SNR=%7.3f dB | NF=%8.3f dBm | CP=%8.3f dBm | file_ms=%8.2f\n",
               i + 1, m.snr_center_db, m.noise_floor_dbm, m.center_power_dbm, now_ms() - file_start);

        mt_free(&mt, psd, bins * sizeof(float));
        mt_free(&mt, freq, bins * sizeof(float));
        mt_free(&mt, iq, n_complex * sizeof(complexf_t));
        mt_free(&mt, raw, raw_bytes);
    }

    t.total_ms = now_ms() - total_start;
    double rss_end = process_rss_mb();

    int n = files_to_use;
    double mean_noise = (n > 0) ? (sum_noise / n) : 0.0;
    double mean_center = (n > 0) ? (sum_center / n) : 0.0;
    double mean_snr = (n > 0) ? (sum_snr / n) : 0.0;
    double var_snr = (n > 1) ? ((sum_snr2 - (sum_snr * sum_snr) / n) / (n - 1)) : 0.0;
    if (var_snr < 0.0)
    {
        var_snr = 0.0;
    }

    double score = mean_snr - 2.0 * sqrt(var_snr) - (mean_noise + 90.0);
    double throughput = (t.total_ms > 0.0) ? (total_samples / (t.total_ms / 1000.0)) : 0.0;

    printf("\n================================================================================\n");
    printf("SUMMARY\n");
    printf("================================================================================\n");
    printf("total_ms          : %.3f\n", t.total_ms);
    printf("load_ms           : %.3f\n", t.load_ms);
    printf("convert_ms        : %.3f\n", t.convert_ms);
    printf("preprocess_ms     : %.3f\n", t.preprocess_ms);
    printf("welch_ms          : %.3f\n", t.welch_ms);
    printf("metric_ms         : %.3f\n", t.metric_ms);
    printf("samples_total     : %.0f\n", total_samples);
    printf("throughput_sps    : %.2f\n", throughput);
    printf("snr_mean_db       : %.4f\n", mean_snr);
    printf("snr_std_db        : %.4f\n", sqrt(var_snr));
    printf("noise_mean_dbm    : %.4f\n", mean_noise);
    printf("center_mean_dbm   : %.4f\n", mean_center);
    printf("score             : %.4f\n", score);
    printf("rss_start_mb      : %.3f\n", rss_start);
    printf("rss_end_mb        : %.3f\n", rss_end);
    printf("rss_delta_mb      : %.3f\n", rss_end - rss_start);

    printf("\nComponent share:\n");
    printf("- Load      %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.load_ms / t.total_ms) : 0.0);
    printf("- Convert   %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.convert_ms / t.total_ms) : 0.0);
    printf("- Preproc   %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.preprocess_ms / t.total_ms) : 0.0);
    printf("- Welch     %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.welch_ms / t.total_ms) : 0.0);
    printf("- Metric    %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.metric_ms / t.total_ms) : 0.0);

    printf("\nMemory tracker:\n");
    printf("- alloc_count      : %llu\n", (unsigned long long)mt.alloc_count);
    printf("- free_count       : %llu\n", (unsigned long long)mt.free_count);
    printf("- bytes_peak       : %llu\n", (unsigned long long)mt.bytes_peak);
    printf("- bytes_current    : %llu\n", (unsigned long long)mt.bytes_current);

    if (mt.bytes_current != 0 || mt.alloc_count != mt.free_count)
    {
        /* Internal tracker is advisory; use external tooling for deep leak checks. */
        printf("[LEAK WARNING] Potential memory leak detected in internal allocation tracking.\n");
    }
    else
    {
        printf("[OK] Internal allocation tracker reports no leaks.\n");
    }

    return 0;
}
