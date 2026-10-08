#include "forgeefx_host_services.h"

static int process(int sample, int *params, int *state)
{
    (void)params;
    (void)state;
    return sample;
}

static void reset(int *state) { state[0] = 0; }
static const ForgeEFXParameter parameter = { "gain", "Gain", "", 0, 100, 100, 0, 0, 0 };
static const ForgeEFXBlockApi api = {
    FORGEEFX_BLOCK_ABI_VERSION, sizeof(ForgeEFXBlockApi),
    "test", "test.host_services", "Host services fixture", "1.0.0", "test", "Test", "Test",
    0, 0, -1, 1, &parameter, 1, 4, FORGEEFX_BLOCK_SAMPLE_RATE,
    -1, -1, -1, 0, process, reset, 0, 0
};

const ForgeEFXBlockApi *forgeefx_get_block_api(uint32_t requested_abi)
{
    return requested_abi == FORGEEFX_BLOCK_ABI_VERSION ? &api : 0;
}

#ifndef FORGEEFX_LEGACY_FIXTURE
/* Test-only export calls the real module-local helper without opening an editor. */
FORGEEFX_BLOCK_EXPORT int32_t forgeefx_test_query_folder(uint32_t folder, char *buffer,
    uint32_t buffer_size, uint32_t *required_size)
{
    return forgeefx_host_get_folder(folder, buffer, buffer_size, required_size);
}
#endif
