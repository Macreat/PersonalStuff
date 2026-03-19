#include "iq_bench.h"

/*
 * fs_utils.c
 * Discovers .sigmf-data/.sigmf-meta pairs in local DB directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

static int has_suffix(const char *s, const char *suffix)
{
    /* Tiny helper to filter only expected file types. */
    size_t ls = strlen(s);
    size_t lx = strlen(suffix);
    if (ls < lx)
    {
        return 0;
    }
    return strcmp(s + (ls - lx), suffix) == 0;
}

static int pair_cmp(const void *a, const void *b)
{
    /* Stable lexical sort for deterministic benchmark order. */
    const sigmf_pair_t *pa = (const sigmf_pair_t *)a;
    const sigmf_pair_t *pb = (const sigmf_pair_t *)b;
    return strcmp(pa->data_path, pb->data_path);
}

int discover_sigmf_pairs(const char *db_dir, sigmf_pair_t *out_pairs, int max_pairs)
{
    /* Windows implementation only in this project variant. */
    int count = 0;

#ifdef _WIN32
    char pattern[MAX_PATH_LEN];
    snprintf(pattern, sizeof(pattern), "%s\\*.sigmf-data", db_dir);

    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(pattern, &ffd);
    if (h == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    do
    {
        if (count >= max_pairs)
        {
            break;
        }
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        if (!has_suffix(ffd.cFileName, ".sigmf-data"))
        {
            continue;
        }

        char data_path[MAX_PATH_LEN];
        char meta_path[MAX_PATH_LEN];
        snprintf(data_path, sizeof(data_path), "%s\\%s", db_dir, ffd.cFileName);

        strncpy(meta_path, data_path, sizeof(meta_path) - 1);
        meta_path[sizeof(meta_path) - 1] = '\0';

        char *p = strstr(meta_path, ".sigmf-data");
        if (!p)
        {
            continue;
        }
        strcpy(p, ".sigmf-meta");

        DWORD attrs = GetFileAttributesA(meta_path);
        if (attrs == INVALID_FILE_ATTRIBUTES)
        {
            continue;
        }

        strncpy(out_pairs[count].data_path, data_path, MAX_PATH_LEN - 1);
        out_pairs[count].data_path[MAX_PATH_LEN - 1] = '\0';
        strncpy(out_pairs[count].meta_path, meta_path, MAX_PATH_LEN - 1);
        out_pairs[count].meta_path[MAX_PATH_LEN - 1] = '\0';
        count++;

    } while (FindNextFileA(h, &ffd));

    FindClose(h);
#else
    (void)db_dir;
    (void)out_pairs;
    (void)max_pairs;
    return 0;
#endif

    qsort(out_pairs, (size_t)count, sizeof(sigmf_pair_t), pair_cmp);
    return count;
}
