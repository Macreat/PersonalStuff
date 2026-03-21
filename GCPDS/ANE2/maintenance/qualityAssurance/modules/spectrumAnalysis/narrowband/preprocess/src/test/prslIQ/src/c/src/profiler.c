#include "iq_bench.h"

/**
 * @file profiler.c
 * @brief Utilidades de tiempo, memoria de proceso y tracking de asignaciones.
 */

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/**
 * @brief Retorna tiempo monotono en milisegundos.
 * @return Tiempo en ms.
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

/**
 * @brief Obtiene RSS actual del proceso.
 * @return Memoria RSS en MB.
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

/**
 * @brief Obtiene RSS pico reportado por el sistema operativo.
 * @return Memoria RSS pico en MB.
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

/**
 * @brief Retorna porcentaje de carga de RAM del sistema.
 * @return Porcentaje de RAM en uso.
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

/**
 * @brief Inicializa contadores del tracker interno.
 * @param mt Tracker de memoria.
 */
void mem_tracker_init(mem_tracker_t *mt)
{
    /* Internal tracker, not a replacement for ASan/Valgrind. */
    mt->alloc_count = 0;
    mt->free_count = 0;
    mt->bytes_current = 0;
    mt->bytes_peak = 0;
}

/**
 * @brief Reserva memoria y actualiza contadores del tracker.
 * @param mt Tracker de memoria.
 * @param bytes Cantidad de bytes a reservar.
 * @return Puntero reservado o NULL.
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

/**
 * @brief Libera memoria y actualiza contadores del tracker.
 * @param mt Tracker de memoria.
 * @param ptr Puntero a liberar.
 * @param bytes Bytes asociados a la reserva original.
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
