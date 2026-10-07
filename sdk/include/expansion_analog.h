#ifndef RETROEFX_EXPANSION_ANALOG_H
#define RETROEFX_EXPANSION_ANALOG_H

/* Small, independent DSP primitives. State is always caller owned; float
 * histories cross the integer host ABI only through memcpy. */
#include <math.h>
#include <stdint.h>
#include <string.h>
#define AX_PI 3.14159265358979323846f
#define AX_RATE 48000.0f
/* MOTU guitar input at +0 dB, calibrated against the measured BOSS BD-2
 * (6 October 2026). These are peak sine volts per digital full scale, not RMS.
 * Convert only at the circuit's jacks; internal gains and rails stay in volts. */
#define AX_IN_VOLTS 5.62f
#define AX_OUT_VOLTS 4.04f
#define AX_WORDS(type) ((sizeof(type) + sizeof(int) - 1) / sizeof(int))
#define AX_GUARD(type) typedef char AxCapacity[(AX_WORDS(type) <= 32772) ? 1 : -1]
typedef char AxFloatWordSize[(sizeof(float) == sizeof(int)) ? 1 : -1];

static float axClamp(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
static float axKnob(int x) { return axClamp(x * 0.01f, 0, 1); }
static float axInput(int x) { return axClamp(x / 65536.0f, -1, 0.999984741f); }
static int axOutput(float x) {
    if (!isfinite(x)) return 0;
    return (int)(axClamp(x, -1, 0.999984741f) * 65536.0f);
}
static float axDb(float db) { return powf(10.0f, db * 0.05f); }
static float axExp(float lo, float hi, float t) { return lo * powf(hi / lo, axClamp(t, 0, 1)); }
static float axMix(float dry, float wet, float mix) { return dry + mix * (wet - dry); }
static float axSaturate(float x) { return tanhf(x); }
static float axPole(float hz) { return 1.0f - expf(-2.0f * AX_PI * axClamp(hz, 1, 20000) / AX_RATE); }
static float axLow(float x, float *z, float coefficient) { *z += coefficient * (x - *z); return *z; }
static float axDc(float x, float *lastX, float *lastY) {
    float y = x - *lastX + 0.99738543f * *lastY;
    *lastX = x; *lastY = y; return y;
}
static float axPhase(float *phase, float hz) {
    *phase += axClamp(hz, 0, 12000) / AX_RATE;
    *phase -= floorf(*phase); return *phase;
}
static float axTriangle(float p) { return 1.0f - 4.0f * fabsf(p - 0.5f); }
static uint32_t axRandom(uint32_t *seed) {
    if (!*seed) *seed = 0x514E3u;
    *seed ^= *seed << 13; *seed ^= *seed >> 17; *seed ^= *seed << 5;
    return *seed;
}
static float axWave(float p, int shape) {
    switch (shape) {
        case 1: return axTriangle(p);
        case 2: return p < 0.5f ? 1.0f : -1.0f;
        case 3: return 2.0f * p - 1.0f;
        case 4: return 1.0f - 2.0f * p;
        default: return sinf(2 * AX_PI * p);
    }
}
/* PolyBLEP suppresses the largest discontinuity aliases of the free oscillator. */
static float axBlep(float t, float dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1; }
    if (t > 1 - dt) { t = (t - 1) / dt; return t * t + t + t + 1; }
    return 0;
}
typedef struct AxSvf { float z1, z2; } AxSvf;
static float axFilter(float x, AxSvf *s, float hz, float q, int lowpass) {
    float g = tanf(AX_PI * axClamp(hz, 15, 15000) / AX_RATE);
    float k = 1.0f / axClamp(q, 0.3f, 20);
    float a = 1.0f / (1 + g * (g + k));
    float v1 = a * (s->z1 + g * (x - s->z2));
    float v2 = s->z2 + g * v1;
    s->z1 = 2 * v1 - s->z1; s->z2 = 2 * v2 - s->z2;
    return lowpass ? v2 : v1 * k;
}
static float axBand(float x, AxSvf *s, float hz, float q) { return axFilter(x,s,hz,q,0); }
typedef struct AxBiquad { float b0,b1,b2,a1,a2,z1,z2; } AxBiquad;
static void axPeak(AxBiquad *s, float hz, float q, float db) {
    float a = powf(10, db / 40), w = 2 * AX_PI * hz / AX_RATE;
    float alpha = sinf(w) / (2 * q), c = cosf(w), inverse = 1 / (1 + alpha / a);
    s->b0 = (1 + alpha * a) * inverse; s->b1 = -2 * c * inverse;
    s->b2 = (1 - alpha * a) * inverse; s->a1 = s->b1;
    s->a2 = (1 - alpha / a) * inverse;
}
static void axShelf(AxBiquad *s, float hz, float db, int high) {
    float a = powf(10, db / 40), w = 2 * AX_PI * hz / AX_RATE;
    float c = cosf(w), r = sqrtf(2 * a) * sinf(w), ap = a + 1, am = a - 1;
    float a0;
    if (high) {
        a0 = ap - am * c + r;
        s->b0 = a * (ap + am * c + r) / a0;
        s->b1 = -2 * a * (am + ap * c) / a0;
        s->b2 = a * (ap + am * c - r) / a0;
        s->a1 = 2 * (am - ap * c) / a0;
        s->a2 = (ap - am * c - r) / a0;
    } else {
        a0 = ap + am * c + r;
        s->b0 = a * (ap - am * c + r) / a0;
        s->b1 = 2 * a * (am - ap * c) / a0;
        s->b2 = a * (ap - am * c - r) / a0;
        s->a1 = -2 * (am + ap * c) / a0;
        s->a2 = (ap + am * c - r) / a0;
    }
}
static float axBiquad(float x, AxBiquad *s) {
    float y = s->b0*x + s->z1;
    s->z1 = s->b1*x - s->a1*y + s->z2;
    s->z2 = s->b2*x - s->a2*y; return y;
}
static float axAllpass(float x, float *z, float coefficient) {
    float y = coefficient * x + *z;
    *z = x - coefficient * y; return y;
}
static float axPhaser(float x, float *z, int count, float hz) {
    float t = tanf(AX_PI * axClamp(hz, 25, 12000) / AX_RATE);
    float coefficient = (t - 1) / (t + 1);
    int stage;
    for (stage = 0; stage < count; ++stage) x = axAllpass(x, &z[stage], coefficient);
    return x;
}
/* A delay buffer lives after a small state header, never copied per sample. */
static float axLoad(const int *state, int offset) { float value; memcpy(&value, state + offset, sizeof(value)); return value; }
static void axStore(int *state, int offset, float value) { memcpy(state + offset, &value, sizeof(value)); }
static float axDelay(const int *state, int base, int size, int write, float delay) {
    float position = write - axClamp(delay, 1, (float)size - 3);
    int i; float fraction, a, b;
    if (position < 0) position += size;
    i = (int)position; fraction = position - i;
    a = axLoad(state, base + i); b = axLoad(state, base + ((i + 1) % size));
    return a + (b - a) * fraction;
}
#endif
