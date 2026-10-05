#include <stdint.h>
void gain_reset(int *state) { state[0] = 0; }
int gain_process(int sample, int *params, int *state)
{
    int64_t output = (int64_t) sample * params[0] / 100;
    (void) state;
    return output < -65536 ? -65536 : output > 65535 ? 65535 : (int) output;
}
