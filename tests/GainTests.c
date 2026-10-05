#include <stdio.h>

int gain_process(int sample, int *params, int *state);
void gain_reset(int *state);

int main(void)
{
    int state[1] = {123};
    int params[64] = {100};
    gain_reset(state);
    if (state[0] != 0 || gain_process(12345, params, state) != 12345 ||
        gain_process(-12345, params, state) != -12345) {
        fputs("Reset or unity gain failed\n", stderr);
        return 1;
    }
    params[0] = 0;
    if (gain_process(65535, params, state) != 0) {
        fputs("Zero gain failed\n", stderr);
        return 1;
    }
    params[0] = 200;
    if (gain_process(12345, params, state) != 24690 ||
        gain_process(65535, params, state) != 65535 ||
        gain_process(-65536, params, state) != -65536) {
        fputs("Boost or saturation failed\n", stderr);
        return 1;
    }
    return 0;
}
