#include "iq_bench.h"

/*
 * profiler.c
 * Timing, process RSS sampling, and simple internal allocation accounting.
 */

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * now_ms
 * Retorna tiempo monotono en milisegundos para medir etapas del pipeline.
 */
double now_ms(void)
{
    /* High-resolution monotonic timer for stage profiling. */
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER t;
    if (freq.QuadPart == 0)
    {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    return 0.0;
#endif
}

/*
 * process_rss_mb
 * Snapshot de RSS actual del proceso (MB).
 */
double process_rss_mb(void)
{
    /* Working set snapshot; useful as coarse memory trend signal. */
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
    }
#endif
    return 0.0;
}

/*
 * process_peak_rss_mb
 * Retorna RSS pico observado por el SO para el proceso actual (MB).
 */
double process_peak_rss_mb(void)
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return (double)pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
    }
#endif
    return 0.0;
}

/*
 * system_ram_percent
 * Retorna porcentaje de carga de RAM del sistema reportado por el SO.
 */
double system_ram_percent(void)
{
#ifdef _WIN32
    MEMORYSTATUSEX st;
    st.dwLength = sizeof(st);
    if (GlobalMemoryStatusEx(&st))
    {
        return (double)st.dwMemoryLoad;
    }
#endif
    return 0.0;
}

/*
 * mem_tracker_init
 * Inicializa contadores del tracker interno de asignaciones.
 */
void mem_tracker_init(mem_tracker_t *mt)
{
    /* Internal tracker, not a replacement for ASan/Valgrind. */
    mt->alloc_count = 0;
    mt->free_count = 0;
    mt->bytes_current = 0;
    mt->bytes_peak = 0;
}

/*
 * mt_alloc
 * Envoltura de malloc con contabilidad de bytes y cantidad de allocs.
 */
void *mt_alloc(mem_tracker_t *mt, size_t bytes)
{
    /* Wrapper keeps allocation counters centralized. */
    void *p = malloc(bytes);
    if (!p)
    {
        return NULL;
    }
    mt->alloc_count += 1;
    mt->bytes_current += bytes;
    if (mt->bytes_current > mt->bytes_peak)
    {
        mt->bytes_peak = mt->bytes_current;
    }
    return p;
}

/*
 * mt_free
 * Envoltura de free con contabilidad de bytes liberados.
 * Requiere que bytes coincida con la reserva original para audit correcto.
 */
void mt_free(mem_tracker_t *mt, void *ptr, size_t bytes)
{
    /* Caller must pass the same size used at allocation for accurate accounting. */
    if (!ptr)
    {
        return;
    }
    free(ptr);
    mt->free_count += 1;
    if (mt->bytes_current >= bytes)
    {
        mt->bytes_current -= bytes;
    }
    else
    {
        mt->bytes_current = 0;
    }
}
