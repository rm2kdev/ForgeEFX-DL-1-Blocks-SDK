#ifndef FORGEEFX_BLOCK_H
#define FORGEEFX_BLOCK_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "block_ui.h"
#define FORGEEFX_BLOCK_ABI_VERSION 1u
#define FORGEEFX_BLOCK_SAMPLE_RATE 48000u
#define FORGEEFX_BLOCK_MAX_PARAMETERS 64u
#if !defined(FORGEEFX_BUILD_MODULE)
#define FORGEEFX_BLOCK_EXPORT
#elif defined(_WIN32)
#define FORGEEFX_BLOCK_EXPORT __declspec(dllexport)
#else
#define FORGEEFX_BLOCK_EXPORT __attribute__((visibility("default")))
#endif
/* ABI v1 requires a 32-bit int and natural platform alignment. All strings and
   tables belong to the module and remain valid until it is unloaded. */
typedef struct ForgeEFXParameter {
    const char *key;
    const char *name;
    const char *unit;
    int32_t minimum;
    int32_t maximum;
    int32_t default_value;
    int32_t format; /* 0 number, 1 knob, 2 signed, 3 choice */
    uint32_t choice_count;
    const char *const *choices;
} ForgeEFXParameter;
typedef struct ForgeEFXRenderServices {
    uint32_t abi_version;
    uint32_t struct_size;
#define FORGEEFX_RENDER_SERVICE(result, name, args) result (*name) args;
#include "forgeefx_render_services.inc"
#undef FORGEEFX_RENDER_SERVICE
} ForgeEFXRenderServices;
typedef struct ForgeEFXBlockApi {
    uint32_t abi_version;
    uint32_t struct_size;
    const char *developer_id;
    const char *effect_id;
    const char *name;
    const char *package_version;
    const char *category_id;
    const char *category_name;
    const char *category_short_name;
    int32_t category_order;
    int32_t effect_order;
    int32_t legacy_index; /* -1 for new effects */
    uint32_t parameter_count;
    const ForgeEFXParameter *parameters;
    uint32_t state_words; /* per channel; zero initialized by host before reset */
    uint32_t state_alignment;
    uint32_t sample_rate;
    int32_t tempo_param;
    int32_t sync_param;
    int32_t trails_param;
    int32_t tail_seconds;
    /* Parameter order is sample, params, state, matching original firmware. */
    int (*process)(int sample, int *params, int *state);
    void (*reset)(int *state);
    void (*stereo_process)(int left, int right, int *params, int *left_state,
                           int *right_state, int *left_output, int *right_output);
    /* Message thread only; services and ui are valid for this call only.
       Null render requests the host's default parameter editor. */
    void (*render)(const BlockUI *ui, const ForgeEFXRenderServices *services);
} ForgeEFXBlockApi;
typedef const ForgeEFXBlockApi *(*ForgeEFXGetBlockApi)(uint32_t requested_abi);
FORGEEFX_BLOCK_EXPORT const ForgeEFXBlockApi *forgeefx_get_block_api(uint32_t requested_abi);
/* Module-local bridge saves/restores a TLS service table for nested renders. */
const ForgeEFXRenderServices *forgeefx_set_render_services(const ForgeEFXRenderServices *services);
#ifdef __cplusplus
}
#endif
#endif
