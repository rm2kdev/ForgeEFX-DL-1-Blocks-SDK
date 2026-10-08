#ifndef FORGEEFX_HOST_SERVICES_H
#define FORGEEFX_HOST_SERVICES_H

#include "forgeefx_block.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Optional extension: the block and rendering ABI v1 layouts are unchanged. */
#define FORGEEFX_HOST_SERVICES_ABI_VERSION 1u
#define FORGEEFX_FOLDER_NAM 1u
#define FORGEEFX_FOLDER_IR 2u

#define FORGEEFX_HOST_OK 0
#define FORGEEFX_HOST_UNAVAILABLE 1
#define FORGEEFX_HOST_INVALID_ARGUMENT 2
#define FORGEEFX_HOST_BUFFER_TOO_SMALL 3
#define FORGEEFX_HOST_ERROR 4

typedef struct ForgeEFXHostServices {
    uint32_t abi_version;
    uint32_t struct_size;
    void *context;
    /* UI/worker threads only; never call from process/reset. Returns the current
       absolute folder as UTF-8, including its terminating NUL. required_size is
       mandatory and counts bytes including NUL. A NULL buffer with size 0 is a
       size query; it returns BUFFER_TOO_SMALL with the required byte count.
       Other errors set required_size to 0. Non-OK results empty a supplied
       nonempty buffer; truncated paths are never returned. */
    int32_t (*get_folder)(void *context, uint32_t folder, char *buffer,
                          uint32_t buffer_size, uint32_t *required_size);
} ForgeEFXHostServices;

typedef void (*ForgeEFXSetHostServices)(const ForgeEFXHostServices *services);

/* The host optionally binds once after descriptor validation, before callbacks.
   The table and context must remain valid until all module calls have finished.
   Binding must not race module calls. NULL, unknown versions, short tables and
   absent callbacks disable the extension. Older hosts simply omit this call. */
FORGEEFX_BLOCK_EXPORT void forgeefx_set_host_services(const ForgeEFXHostServices *services);

/* Module-local helper, available without a custom editor. Memory stays owned by
   the caller. UNKNOWN folder IDs/invalid buffers return INVALID_ARGUMENT;
   absent host services return UNAVAILABLE. Query again to observe folder edits.
   Size and copy queries are separate snapshots: retry if the folder grows. */
int32_t forgeefx_host_get_folder(uint32_t folder, char *buffer,
                               uint32_t buffer_size, uint32_t *required_size);

#ifdef __cplusplus
}
#endif
#endif
