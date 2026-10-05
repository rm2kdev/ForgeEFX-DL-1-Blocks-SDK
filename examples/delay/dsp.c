#include <stdint.h>

/* 1000 ms at 48 kHz. Word 0 is the write cursor; the rest is Q16 history.
   Keep state_words in parameters.json equal to DELAY_SAMPLES + 1. */
enum { DELAY_SAMPLES = 48000 };

static int clamp(int value, int minimum, int maximum)
{
    return value < minimum ? minimum : value > maximum ? maximum : value;
}

static int saturate(int64_t value)
{
    return value < -65536 ? -65536 : value > 65535 ? 65535 : (int) value;
}

void delay_reset(int *state)
{
    for (int i = 0; i <= DELAY_SAMPLES; ++i) state[i] = 0;
}

int delay_process(int sample, int *params, int *state)
{
    int delay_samples = clamp(params[0], 1, 1000) * 48;
    int feedback = clamp(params[1], 0, 90);
    int mix = clamp(params[2], 0, 100);
    int cursor = state[0];
    int read = cursor - delay_samples;
    if (read < 0) read += DELAY_SAMPLES;
    int delayed = state[1 + read];

    /* Read before writing so the maximum delay uses the entire buffer.
       Saturate the feedback path as well as the output to bound repeated input. */
    state[1 + cursor] = saturate((int64_t) sample + (int64_t) delayed * feedback / 100);
    state[0] = cursor + 1 == DELAY_SAMPLES ? 0 : cursor + 1;
    return saturate(((int64_t) sample * (100 - mix) + (int64_t) delayed * mix) / 100);
}
