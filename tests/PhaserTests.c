#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int phaser_process(int sample, int *params, int *state);
void phaser_reset(int *state);

#define CHECK(condition, message) do { if (!(condition)) { \
    fputs(message "\n", stderr); return 1; } } while (0)

static double tone_energy(int frequency)
{
    int params[64] = {5, 0};
    int state[5] = {0};
    double energy = 0;
    for (int i = 0; i < 48000; ++i) {
        int input = (int) (12000 * sin(6.283185307179586 * frequency * i / 48000));
        int output = phaser_process(input, params, state);
        if (i >= 24000)
            energy += (double) output * output;
    }
    return energy;
}

int main(void)
{
    int params[64] = {5, 75};
    int left[5] = {0}, right[5] = {0}, reference[5] = {0};
    int saved[4096];
    uint32_t noise = 1;

    for (int rate = 1; rate <= 50; rate += 49) {
        for (int depth = 0; depth <= 100; depth += 100) {
            params[0] = rate;
            params[1] = depth;
            phaser_reset(left);
            for (int i = 0; i < 480000; ++i)
                CHECK(phaser_process(0, params, left) == 0, "Silence generated noise");
        }
    }

    params[0] = 5;
    params[1] = 75;
    phaser_reset(left);
    for (int i = 0; i < 4096; ++i)
        saved[i] = phaser_process(i == 0 ? 32768 : 0, params, left);
    CHECK(saved[0] != 32768 && saved[1] != 0, "No phase filtering");
    CHECK(saved[4095] == 0, "Impulse did not decay");
    phaser_reset(left);
    for (int i = 0; i < 5; ++i)
        CHECK(left[i] == 0, "Incomplete reset");
    for (int i = 0; i < 4096; ++i)
        CHECK(phaser_process(i == 0 ? 32768 : 0, params, left) == saved[i],
              "Reset was not deterministic");

    phaser_reset(left);
    for (int i = 0; i < 480000; ++i) {
        noise = noise * 1664525u + 1013904223u;
        int input = (int) (noise >> 16) * 2 - 65536;
        params[0] = (i & 1) ? 1 : 50;
        params[1] = (i & 2) ? 0 : 100;
        int output = phaser_process(input, params, left);
        CHECK(output >= -65536 && output <= 65535, "Output exceeded Q16 full scale");
        CHECK(phaser_process(0, params, right) == 0, "Stereo channel crosstalk");
        CHECK(phaser_process(input, params, reference) == output, "Instance interference");
    }
    CHECK(phaser_process(INT_MIN, params, left) >= -65536, "Negative overload");
    CHECK(phaser_process(INT_MAX, params, left) <= 65535, "Positive overload");

    /* Four equal all-pass stages at a=0.75 should cancel near 903 Hz,
       while retaining low frequencies. This catches a missing dry blend,
       incorrect all-pass sign, or a pass-through implementation. */
    CHECK(tone_energy(903) < tone_energy(100) * 0.01, "Phaser notch missing");

    phaser_reset(left);
    phaser_reset(right);
    params[0] = 1;
    params[1] = 100;
    int differences = 0;
    for (int i = 0; i < 48000; ++i) {
        int input = (int) (12000 * sin(6.283185307179586 * 903 * i / 48000));
        params[0] = 1;
        int slow = phaser_process(input, params, left);
        params[0] = 50;
        differences += slow != phaser_process(input, params, right);
    }
    CHECK(differences > 24000, "Rate did not change the sweep");
    puts("Phaser silence, reset, tails, extremes, independent channels and notch passed");
    return 0;
}
