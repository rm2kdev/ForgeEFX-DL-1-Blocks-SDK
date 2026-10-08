#ifndef FORGEEFX_MODEL_H
#define FORGEEFX_MODEL_H
#include "forgeefx_block.h"
#ifdef __cplusplus
extern "C" {
#endif
#define FORGEEFX_MODEL_ABI_VERSION 1u
/* Optional model extension; block/render ABI v1 is unchanged.
   The module owns every opaque instance and allocation. prepare/destroy run on
   a worker, never audio/reset. prepare receives an absolute UTF-8 filename and
   returns NULL on failure, writing a NUL-terminated error when capacity > 0.
   A prepared instance contains independent left/right histories at 48 kHz.
   Publish only at an audio block boundary; retire before worker destruction.
   process runs on one audio thread per instance, without allocation, I/O or
   blocking. params points to the block's parameter_count values for this call.
   No callback may throw across this interface. The immutable table and module
   must outlive all instances/callbacks. Different instances may run concurrently.
   Hosts without this extension cannot load packages that require it: declare
   minimum_host_version 0.1.1 or later. Paths/state remain host-owned. */
typedef struct ForgeEFXModelApi {
    uint32_t abi_version;
    uint32_t struct_size;
    void *(*prepare)(const char *path, char *error, uint32_t capacity);
    void (*destroy)(void *instance);
    float (*process)(void *instance, uint32_t channel, float input, const int32_t *params);
} ForgeEFXModelApi;
typedef const ForgeEFXModelApi *(*ForgeEFXGetModelApi)(uint32_t requested_abi);
FORGEEFX_BLOCK_EXPORT const ForgeEFXModelApi *forgeefx_get_model_api(uint32_t requested_abi);
#ifdef __cplusplus
}
#endif
#endif
