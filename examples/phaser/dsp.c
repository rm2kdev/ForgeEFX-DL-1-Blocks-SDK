#include <stdint.h>
#include <limits.h>

/* State: triangle phase followed by four all-pass delay words, per channel. */
static int clamp(int64_t value, int low, int high)
{
    return value < low ? low : value > high ? high : (int) value;
}

void phaser_reset(int *state)
{
    for (int i = 0; i < 5; ++i)
        state[i] = 0;
}

int phaser_process(int sample, int *params, int *state)
{
    const int rate = clamp(params[0], 1, 50); /* Tenths of Hz. */
    const int depth = clamp(params[1], 0, 100);
    const int dry = clamp(sample, -65536, 65535);
    const int phase = state[0];
    const int triangle = phase < 240000 ? phase * 2 - 240000
                                      : 720000 - phase * 2;
    /* Q15 coefficient stays in [0.53125, 0.96875], safely inside unity.
       Depth zero holds the filter at its centre; it is not a bypass. */
    const int coefficient = 24576 + (int) ((int64_t) triangle * depth * 7168
                                         / 24000000);
    int wet = dry;

    state[0] = (phase + rate) % 480000;
    for (int stage = 1; stage <= 4; ++stage) {
        /* y = z - a*x; z = x + a*y. Keep interstage headroom and widen
           before every product/sum, including abrupt parameter changes. */
        const int output = clamp((int64_t) state[stage]
                                 - (int64_t) coefficient * wet / 32768,
                                 INT_MIN, INT_MAX);
        state[stage] = clamp((int64_t) wet
                             + (int64_t) coefficient * output / 32768,
                             INT_MIN, INT_MAX);
        wet = output;
    }
    /* A fixed equal dry/wet blend creates the phaser's cancellation notches. */
    return clamp(((int64_t) dry + wet) / 2, -65536, 65535);
}
