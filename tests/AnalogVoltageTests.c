#include "expansion_analog.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int main(void)
{
    /* Independent measured references: half-scale input is 2.81 V peak;
       2.02 V peak at the output jack is half-scale digital output. */
    CHECK(fabsf(axInput(32768) * AX_IN_VOLTS - 2.81f) < 0.000001f);
    CHECK(fabsf(axInput(-32768) * AX_IN_VOLTS + 2.81f) < 0.000001f);
    CHECK(abs(axOutput(2.02f / AX_OUT_VOLTS) - 32768) <= 1);
    CHECK(abs(axOutput(-2.02f / AX_OUT_VOLTS) + 32768) <= 1);

    /* A wire in a physical circuit preserves volts, giving +2.867 dB in
       digital samples. A mistaken unity trim or double conversion fails. */
    CHECK(abs(axOutput(axInput(16384) * AX_IN_VOLTS / AX_OUT_VOLTS) - 22791) <= 1);
    CHECK(abs(axOutput(axInput(-16384) * AX_IN_VOLTS / AX_OUT_VOLTS) + 22791) <= 1);
    CHECK(fabsf(20.0f * log10f(AX_IN_VOLTS / AX_OUT_VOLTS) - 2.867f) < 0.001f);

    CHECK(axOutput(axInput(0) * AX_IN_VOLTS / AX_OUT_VOLTS) == 0);
    CHECK(axOutput(9.0f / AX_OUT_VOLTS) == 65535);
    CHECK(axOutput(-9.0f / AX_OUT_VOLTS) == -65536);
    CHECK(axOutput(axInput(INT_MAX)) == 65535);
    CHECK(axOutput(axInput(INT_MIN)) == -65536);
    CHECK(axOutput(NAN) == 0);
    CHECK(axOutput(INFINITY) == 0);
    CHECK(axOutput(-INFINITY) == 0);
    return 0;
}
