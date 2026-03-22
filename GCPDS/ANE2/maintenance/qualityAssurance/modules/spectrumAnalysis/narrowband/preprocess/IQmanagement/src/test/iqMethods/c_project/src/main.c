#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern void process(float *iq, int n);

int main(int argc, char **argv) {
    printf("IQ C project skeleton. Replace process() with chosen algorithm.\n");
    int N = 32768;
    float *iq = (float*)malloc(sizeof(float)*2*N);
    if (!iq) return 1;
    // load or generate sample
    for (int i = 0; i < 2*N; ++i) iq[i] = 0.0f;
    struct timespec t0,t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    process(iq, N);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double ms = (t1.tv_sec - t0.tv_sec)*1000.0 + (t1.tv_nsec - t0.tv_nsec)/1e6;
    printf("Elapsed ms: %.3f\n", ms);
    free(iq);
    return 0;
}
