#include <limits.h>
#include <stdio.h>
#include <string.h>

int delay_process(int sample, int *params, int *state);
void delay_reset(int *state);

enum { STATE_WORDS = 48001 };

/* Guard the exact manifest allocation to catch cursor/reset overruns. */
typedef struct Lane {
    int before;
    int state[STATE_WORDS];
    int after;
} Lane;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int main(void)
{
    Lane left = {0}, right = {0};
    int params[64] = {300, 30, 30};
    const int times[] = {1, 300, 1000};
    const int impulses[] = {32768, -32768};
    left.before = right.before = 1234567;
    left.after = right.after = -1234567;

    /* Exact echo timing, both signs, and wrapping the whole circular buffer. */
    for (int t = 0; t < 3; ++t) {
        for (int sign = 0; sign < 2; ++sign) {
            params[0] = times[t]; params[1] = 0; params[2] = 100;
            delay_reset(left.state);
            for (int n = 0; n < 144001; ++n) {
                int input = n == 0 || n == 48001 ? impulses[sign] : 0;
                int expected = n == times[t] * 48 || n == 48001 + times[t] * 48
                    ? impulses[sign] : 0;
                CHECK(delay_process(input, params, left.state) == expected);
            }
        }
    }

    /* A 50% feedback impulse repeats at half amplitude, without cross-talk. */
    params[0] = 1; params[1] = 50; params[2] = 100;
    delay_reset(left.state);
    delay_reset(right.state);
    for (int n = 0; n <= 192; ++n) {
        int expected = n > 0 && n % 48 == 0 ? 32768 / (1 << (n / 48 - 1)) : 0;
        CHECK(delay_process(n == 0 ? 32768 : 0, params, left.state) == expected);
        CHECK(delay_process(0, params, right.state) == 0);
    }

    /* Default dry/wet levels, fully dry, and independent opposite stereo input. */
    params[0] = 300; params[1] = 30; params[2] = 30;
    delay_reset(left.state);
    for (int n = 0; n <= 14400; ++n) {
        CHECK(delay_process(n == 0 ? 10000 : 0, params, left.state)
            == (n == 0 ? 7000 : n == 14400 ? 3000 : 0));
    }
    params[2] = 0;
    CHECK(delay_process(12345, params, left.state) == 12345);
    CHECK(delay_process(-12345, params, left.state) == -12345);
    CHECK(delay_process(INT_MAX, params, left.state) == 65535);
    CHECK(delay_process(INT_MIN, params, left.state) == -65536);
    params[0] = 1; params[1] = 0; params[2] = 100;
    delay_reset(left.state);
    delay_reset(right.state);
    for (int n = 0; n <= 48; ++n) {
        CHECK(delay_process(n == 0 ? 10000 : 0, params, left.state) == (n == 48 ? 10000 : 0));
        CHECK(delay_process(n == 0 ? -20000 : 0, params, right.state) == (n == 48 ? -20000 : 0));
    }

    /* Extremes and abrupt automation remain bounded with independent instances. */
    for (int n = 0; n < 144000; ++n) {
        params[0] = n % 2 ? 1 : 1000;
        params[1] = n % 3 ? 90 : 0;
        params[2] = n % 5 ? 100 : 0;
        int output = delay_process(n % 2 ? INT_MAX : INT_MIN, params, left.state);
        CHECK(output >= -65536 && output <= 65535);
    }
    delay_reset(left.state);
    delay_reset(right.state);
    for (int n = 0; n < STATE_WORDS; ++n) CHECK(left.state[n] == 0);
    for (int n = 0; n < 96001; ++n) {
        params[0] = n % 2 ? 1 : 1000;
        CHECK(delay_process(0, params, left.state) == 0);
        CHECK(delay_process(0, params, right.state) == 0);
    }
    delay_reset(left.state);
    delay_reset(right.state);
    params[0] = 1; params[1] = 90; params[2] = 100;
    for (int n = 0; n < 96000; ++n) {
        int input = n % 127 == 0 ? 4321 : 0;
        CHECK(delay_process(input, params, left.state) == delay_process(input, params, right.state));
    }
    CHECK(memcmp(left.state, right.state, sizeof(left.state)) == 0);

    /* Saturated history at maximum feedback must drain within the declared tail. */
    params[0] = 1000;
    for (int sign = 0; sign < 2; ++sign) {
        delay_reset(left.state);
        for (int n = 0; n < 96000; ++n) {
            int output = delay_process(sign ? -65536 : 65535, params, left.state);
            CHECK(output >= -65536 && output <= 65535);
        }
        for (int n = 0; n < 120 * 48000; ++n) delay_process(0, params, left.state);
        for (int n = 0; n < 48001; ++n) CHECK(delay_process(0, params, left.state) == 0);
    }
    CHECK(left.before == 1234567 && right.before == 1234567);
    CHECK(left.after == -1234567 && right.after == -1234567);
    puts("Delay timing, mix, feedback, wrap, reset, silence, stereo lanes, extremes and tail passed");
    return 0;
}
