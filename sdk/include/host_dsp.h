#ifndef FREEFX_HOST_DSP_H
#define FREEFX_HOST_DSP_H
/* Native-only shim for the target Q16 intrinsic. No host SDK is required. */
static int host_mul_q16(int a, int b)
{
    long long product = (long long)a * b;
    if (product >= 0) return (int)(product / 65536);
    return (int)(-((-product + 65535) / 65536));
}
#define __builtin_ts201_mul_q16 host_mul_q16
#endif
