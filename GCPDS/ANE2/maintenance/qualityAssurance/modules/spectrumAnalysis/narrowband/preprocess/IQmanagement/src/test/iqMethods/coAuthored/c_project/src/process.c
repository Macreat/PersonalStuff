#include <stdio.h>
#include <stdlib.h>

void process(float *iq, int n) {
    // placeholder: simple DC removal
    for (int i = 0; i < n; ++i) {
        float re = iq[2*i]; float im = iq[2*i+1];
        iq[2*i] = re - 0.0f;
        iq[2*i+1] = im - 0.0f;
    }
}
