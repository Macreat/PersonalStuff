#include "iq_bench.h"

/*
 * main.c
 * CLI entrypoint for Method 3 benchmark and audit reporting.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(const char *exe)
{
    /* Keep options minimal and reproducible for benchmark comparisons. */
    printf("Usage:\n");
    printf("  %s [--db <path>] [--files <N>] [--max-complex <N>] [--nperseg <N>] [--overlap <0..0.9>] [--chunk-bytes <N>] [--json-out <file>] [--csv-out <file>]\n", exe);
    printf("\nDefaults:\n");
    printf("  --db D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz\n");
    printf("  --files 6\n");
    printf("  --max-complex 262144\n");
    printf("  --nperseg 512\n");
    printf("  --overlap 0.5\n");
    printf("  --chunk-bytes 65536\n");
}

static int write_json_summary(const char *path,
                              const perf_timing_t *t,
                              double total_samples,
                              double expected_samples,
                              double throughput,
                              double throughput_eff,
                              double drop_rate_pct,
                              double cpu_ms,
                              double cpu_percent,
                              double ram_percent,
                              double rss_peak,
                              double avg_file_ms,
                              double file_jitter_ms,
                              double latency_ratio,
                              double mean_snr,
                              double std_snr,
                              double mean_noise,
                              double mean_center,
                              double score,
                              double rss_start,
                              double rss_end,
                              int files_ok,
                              int files_req)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        return 0;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"files_requested\": %d,\n", files_req);
    fprintf(f, "  \"files_processed\": %d,\n", files_ok);
    fprintf(f, "  \"samples_total\": %.0f,\n", total_samples);
    fprintf(f, "  \"samples_expected\": %.0f,\n", expected_samples);
    fprintf(f, "  \"throughput_sps\": %.6f,\n", throughput);
    fprintf(f, "  \"throughput_efficiency\": %.6f,\n", throughput_eff);
    fprintf(f, "  \"drop_rate_pct\": %.6f,\n", drop_rate_pct);
    fprintf(f, "  \"cpu_ms\": %.6f,\n", cpu_ms);
    fprintf(f, "  \"cpu_percent\": %.6f,\n", cpu_percent);
    fprintf(f, "  \"ram_percent\": %.6f,\n", ram_percent);
    fprintf(f, "  \"rss_peak_mb\": %.6f,\n", rss_peak);
    fprintf(f, "  \"avg_file_ms\": %.6f,\n", avg_file_ms);
    fprintf(f, "  \"file_jitter_ms\": %.6f,\n", file_jitter_ms);
    fprintf(f, "  \"latency_ratio\": %.6f,\n", latency_ratio);
    fprintf(f, "  \"total_ms\": %.6f,\n", t->total_ms);
    fprintf(f, "  \"load_ms\": %.6f,\n", t->load_ms);
    fprintf(f, "  \"convert_ms\": %.6f,\n", t->convert_ms);
    fprintf(f, "  \"preprocess_ms\": %.6f,\n", t->preprocess_ms);
    fprintf(f, "  \"welch_ms\": %.6f,\n", t->welch_ms);
    fprintf(f, "  \"metric_ms\": %.6f,\n", t->metric_ms);
    fprintf(f, "  \"snr_mean_db\": %.6f,\n", mean_snr);
    fprintf(f, "  \"snr_std_db\": %.6f,\n", std_snr);
    fprintf(f, "  \"noise_mean_dbm\": %.6f,\n", mean_noise);
    fprintf(f, "  \"center_mean_dbm\": %.6f,\n", mean_center);
    fprintf(f, "  \"score\": %.6f,\n", score);
    fprintf(f, "  \"rss_start_mb\": %.6f,\n", rss_start);
    fprintf(f, "  \"rss_end_mb\": %.6f\n", rss_end);
    fprintf(f, "}\n");

    fclose(f);
    return 1;
}

static int write_csv_summary(const char *path,
                             const perf_timing_t *t,
                             double total_samples,
                             double expected_samples,
                             double throughput,
                             double throughput_eff,
                             double drop_rate_pct,
                             double cpu_ms,
                             double cpu_percent,
                             double ram_percent,
                             double rss_peak,
                             double avg_file_ms,
                             double file_jitter_ms,
                             double latency_ratio,
                             double mean_snr,
                             double std_snr,
                             double mean_noise,
                             double mean_center,
                             double score,
                             double rss_start,
                             double rss_end,
                             int files_ok,
                             int files_req)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        return 0;
    }

    fprintf(f, "files_requested,files_processed,samples_total,samples_expected,throughput_sps,throughput_efficiency,drop_rate_pct,cpu_ms,cpu_percent,ram_percent,rss_peak_mb,avg_file_ms,file_jitter_ms,latency_ratio,total_ms,load_ms,convert_ms,preprocess_ms,welch_ms,metric_ms,snr_mean_db,snr_std_db,noise_mean_dbm,center_mean_dbm,score,rss_start_mb,rss_end_mb\n");
    fprintf(f, "%d,%d,%.0f,%.0f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
            files_req, files_ok, total_samples, expected_samples, throughput, throughput_eff, drop_rate_pct,
            cpu_ms, cpu_percent, ram_percent, rss_peak, avg_file_ms, file_jitter_ms, latency_ratio,
            t->total_ms, t->load_ms, t->convert_ms, t->preprocess_ms, t->welch_ms, t->metric_ms,
            mean_snr, std_snr, mean_noise, mean_center, score, rss_start, rss_end);

    fclose(f);
    return 1;
}

int main(int argc, char **argv)
{
    /* Defaults tuned for quick local runs; override via CLI. */
    const char *db_dir = "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz";
    int files_to_use = 6;
    size_t max_complex = 262144;
    int nperseg = 512;
    float overlap = 0.5f;
    size_t chunk_bytes = 65536;
    const char *json_out = NULL;
    const char *csv_out = NULL;

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
        else if (strcmp(argv[i], "--chunk-bytes") == 0 && i + 1 < argc)
        {
            chunk_bytes = (size_t)strtoull(argv[++i], NULL, 10);
        }
        else if (strcmp(argv[i], "--json-out") == 0 && i + 1 < argc)
        {
            json_out = argv[++i];
        }
        else if (strcmp(argv[i], "--csv-out") == 0 && i + 1 < argc)
        {
            csv_out = argv[++i];
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

    int exit_code = 0;
    int8_t *raw_buf = NULL;
    complexf_t *iq_buf = NULL;
    welch_workspace_t ws;
    memset(&ws, 0, sizeof(ws));

    size_t raw_capacity = max_complex * 2U;
    raw_buf = (int8_t *)mt_alloc(&mt, raw_capacity);
    iq_buf = (complexf_t *)mt_alloc(&mt, max_complex * sizeof(complexf_t));
    if (!raw_buf || !iq_buf)
    {
        printf("[ERR] Failed to allocate reusable raw/IQ buffers\n");
        exit_code = 2;
        goto cleanup_all;
    }

    if (!welch_workspace_init(&ws, nperseg, overlap, &mt))
    {
        printf("[ERR] Welch workspace init failed (nperseg must be power-of-two and allocations must succeed)\n");
        exit_code = 2;
        goto cleanup_all;
    }

    perf_timing_t t = {0};
    double total_start = now_ms();
    double rss_start = process_rss_mb();
    clock_t cpu_start = clock();

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
    printf("max_complex=%llu nperseg=%d overlap=%.2f chunk_bytes=%llu\n\n", (unsigned long long)max_complex, nperseg, overlap, (unsigned long long)chunk_bytes);

    int files_processed = 0;
    double sum_file_ms = 0.0;
    double sum_file_ms2 = 0.0;
    double first_sample_rate = 0.0;

    for (int i = 0; i < files_to_use; i++)
    {
        /* Single cleanup path per file stage, even with reusable buffers. */
        sigmf_meta_t meta = {0};
        size_t raw_bytes = 0;
        const float *freq = NULL;
        const float *psd = NULL;
        size_t bins = 0;
        iq_metrics_t m = {0};
        int file_ok = 1;

        double file_start = now_ms();

        if (!parse_sigmf_meta(pairs[i].meta_path, &meta))
        {
            printf("[WARN] Meta parse failed: %s\n", pairs[i].meta_path);
            file_ok = 0;
            goto file_cleanup;
        }

        if (first_sample_rate <= 0.0)
        {
            first_sample_rate = (double)meta.sample_rate;
        }

        double s = now_ms();
        if (!load_iq_int8_limited_chunked_into(pairs[i].data_path, raw_buf, raw_capacity, &raw_bytes, chunk_bytes))
        {
            printf("[WARN] Data load failed: %s\n", pairs[i].data_path);
            file_ok = 0;
            goto file_cleanup;
        }
        t.load_ms += now_ms() - s;

        size_t n_complex = raw_bytes / 2U;
        if (n_complex == 0 || n_complex > max_complex)
        {
            printf("[WARN] Invalid sample count for: %s\n", pairs[i].data_path);
            file_ok = 0;
            goto file_cleanup;
        }
        total_samples += (double)n_complex;

        s = now_ms();
        int8_to_complexf(raw_buf, raw_bytes, iq_buf, n_complex);
        t.convert_ms += now_ms() - s;

        s = now_ms();
        method3_dc_rms_norm(iq_buf, n_complex);
        t.preprocess_ms += now_ms() - s;

        s = now_ms();
        if (!welch_fft_shifted(iq_buf, n_complex, meta.sample_rate, &ws, &freq, &psd, &bins))
        {
            printf("[WARN] Welch failed for: %s\n", pairs[i].data_path);
            file_ok = 0;
            goto file_cleanup;
        }
        t.welch_ms += now_ms() - s;

        s = now_ms();
        compute_metrics_from_psd(freq, psd, bins, &m);
        t.metric_ms += now_ms() - s;

        sum_noise += m.noise_floor_dbm;
        sum_center += m.center_power_dbm;
        sum_snr += m.snr_center_db;
        sum_snr2 += m.snr_center_db * m.snr_center_db;
        files_processed++;
        double file_ms = now_ms() - file_start;
        sum_file_ms += file_ms;
        sum_file_ms2 += file_ms * file_ms;

        printf("%02d) SNR=%7.3f dB | NF=%8.3f dBm | CP=%8.3f dBm | file_ms=%8.2f\n",
               i + 1, m.snr_center_db, m.noise_floor_dbm, m.center_power_dbm, file_ms);

    file_cleanup:
        if (!file_ok)
        {
            printf("[INFO] File skipped: %s\n", pairs[i].data_path);
        }
    }

    t.total_ms = now_ms() - total_start;
    double rss_end = process_rss_mb();
    clock_t cpu_end = clock();
    double cpu_ms = ((double)(cpu_end - cpu_start) * 1000.0) / (double)CLOCKS_PER_SEC;

    int n = files_processed;
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
    double cpu_percent = (t.total_ms > 0.0) ? (100.0 * cpu_ms / t.total_ms) : 0.0;
    double expected_samples = (double)files_to_use * (double)max_complex;
    double throughput_eff = (expected_samples > 0.0) ? (total_samples / expected_samples) : 0.0;
    double drop_rate_pct = (expected_samples > 0.0) ? (100.0 * (expected_samples - total_samples) / expected_samples) : 0.0;
    if (drop_rate_pct < 0.0)
    {
        drop_rate_pct = 0.0;
    }
    double avg_file_ms = (files_processed > 0) ? (sum_file_ms / (double)files_processed) : 0.0;
    double var_file_ms = (files_processed > 1) ? ((sum_file_ms2 - (sum_file_ms * sum_file_ms) / (double)files_processed) / (double)(files_processed - 1)) : 0.0;
    if (var_file_ms < 0.0)
    {
        var_file_ms = 0.0;
    }
    double file_jitter_ms = sqrt(var_file_ms);
    double chunk_duration_ms = (first_sample_rate > 0.0) ? ((1000.0 * (double)max_complex) / first_sample_rate) : 0.0;
    double latency_ratio = (chunk_duration_ms > 0.0) ? (avg_file_ms / chunk_duration_ms) : 0.0;
    double ram_percent = system_ram_percent();
    double rss_peak = process_peak_rss_mb();

    printf("\n================================================================================\n");
    printf("SUMMARY\n");
    printf("================================================================================\n");
    printf("total_ms          : %.3f\n", t.total_ms);
    printf("load_ms           : %.3f\n", t.load_ms);
    printf("convert_ms        : %.3f\n", t.convert_ms);
    printf("preprocess_ms     : %.3f\n", t.preprocess_ms);
    printf("welch_ms          : %.3f\n", t.welch_ms);
    printf("metric_ms         : %.3f\n", t.metric_ms);
    printf("files_processed   : %d\n", files_processed);
    printf("samples_total     : %.0f\n", total_samples);
    printf("throughput_sps    : %.2f\n", throughput);
    printf("throughput_eff    : %.2f %%\n", throughput_eff * 100.0);
    printf("drop_rate_pct     : %.4f %%\n", drop_rate_pct);
    printf("cpu_ms            : %.3f\n", cpu_ms);
    printf("cpu_percent       : %.2f\n", cpu_percent);
    printf("ram_percent       : %.2f %%\n", ram_percent);
    printf("rss_peak_mb       : %.3f\n", rss_peak);
    printf("avg_file_ms       : %.3f\n", avg_file_ms);
    printf("file_jitter_ms    : %.3f\n", file_jitter_ms);
    printf("latency_ratio     : %.3f x_chunk\n", latency_ratio);
    printf("snr_mean_db       : %.4f\n", mean_snr);
    printf("snr_std_db        : %.4f\n", sqrt(var_snr));
    printf("noise_mean_dbm    : %.4f\n", mean_noise);
    printf("center_mean_dbm   : %.4f\n", mean_center);
    printf("score             : %.4f\n", score);
    printf("rss_start_mb      : %.3f\n", rss_start);
    printf("rss_end_mb        : %.3f\n", rss_end);
    printf("rss_delta_mb      : %.3f\n", rss_end - rss_start);

    /* Exact key block for terminal copy/paste and downstream QA parsers. */
    printf("\nCOMPUTATIONAL_METRICS\n");
    printf("total_ms: %.4f\n", t.total_ms);
    printf("throughput_sps: %.5f\n", throughput);
    printf("cpu_percent: %.6f\n", cpu_percent);
    printf("ram_percent: %.1f\n", ram_percent);
    printf("rss_peak_mb: %.6f\n", rss_peak);
    printf("throughput_efficiency: %.6f\n", throughput_eff);
    printf("drop_rate_pct: %.6f\n", drop_rate_pct);
    printf("avg_file_ms: %.5f\n", avg_file_ms);
    printf("file_jitter_ms: %.5f\n", file_jitter_ms);
    printf("latency_ratio: %.5f\n", latency_ratio);
    printf("snr_mean_db: %.6f\n", mean_snr);

    printf("\nComponent share:\n");
    printf("- Load      %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.load_ms / t.total_ms) : 0.0);
    printf("- Convert   %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.convert_ms / t.total_ms) : 0.0);
    printf("- Preproc   %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.preprocess_ms / t.total_ms) : 0.0);
    printf("- Welch     %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.welch_ms / t.total_ms) : 0.0);
    printf("- Metric    %8.2f %%\n", (t.total_ms > 0.0) ? (100.0 * t.metric_ms / t.total_ms) : 0.0);

    if (json_out)
    {
        if (!write_json_summary(json_out, &t, total_samples, expected_samples, throughput, throughput_eff, drop_rate_pct, cpu_ms, cpu_percent, ram_percent, rss_peak, avg_file_ms, file_jitter_ms, latency_ratio, mean_snr, sqrt(var_snr), mean_noise, mean_center, score, rss_start, rss_end, files_processed, files_to_use))
        {
            printf("[WARN] Failed to write JSON summary: %s\n", json_out);
        }
    }

    if (csv_out)
    {
        if (!write_csv_summary(csv_out, &t, total_samples, expected_samples, throughput, throughput_eff, drop_rate_pct, cpu_ms, cpu_percent, ram_percent, rss_peak, avg_file_ms, file_jitter_ms, latency_ratio, mean_snr, sqrt(var_snr), mean_noise, mean_center, score, rss_start, rss_end, files_processed, files_to_use))
        {
            printf("[WARN] Failed to write CSV summary: %s\n", csv_out);
        }
    }

cleanup_all:
    welch_workspace_free(&ws, &mt);
    if (iq_buf)
    {
        mt_free(&mt, iq_buf, max_complex * sizeof(complexf_t));
    }
    if (raw_buf)
    {
        mt_free(&mt, raw_buf, raw_capacity);
    }

    printf("\nMemory tracker:\n");
    printf("- alloc_count      : %llu\n", (unsigned long long)mt.alloc_count);
    printf("- free_count       : %llu\n", (unsigned long long)mt.free_count);
    printf("- bytes_peak       : %llu\n", (unsigned long long)mt.bytes_peak);
    printf("- bytes_current    : %llu\n", (unsigned long long)mt.bytes_current);

    if (mt.bytes_current != 0 || mt.alloc_count != mt.free_count)
    {
        printf("[LEAK BLOCKER] Allocation tracker mismatch detected.\n");
        if (exit_code == 0)
        {
            exit_code = 3;
        }
    }
    else
    {
        printf("[OK] Internal allocation tracker reports no leaks.\n");
    }

    return exit_code;
}
