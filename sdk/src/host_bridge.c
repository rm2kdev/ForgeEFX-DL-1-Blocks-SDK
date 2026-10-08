#include "forgeefx_host_services.h"
#include <stddef.h>

static const ForgeEFXHostServices *hostServices;

void forgeefx_set_host_services(const ForgeEFXHostServices *services)
{
    hostServices = NULL;
    if (!services || services->abi_version != FORGEEFX_HOST_SERVICES_ABI_VERSION
        || services->struct_size < offsetof(ForgeEFXHostServices, get_folder) + sizeof(services->get_folder)
        || !services->get_folder)
        return;
    hostServices = services;
}

int32_t forgeefx_host_get_folder(uint32_t folder, char *buffer,
                               uint32_t buffer_size, uint32_t *required_size)
{
    if (buffer && buffer_size)
        buffer[0] = '\0';
    if (required_size)
        *required_size = 0;
    if (!required_size || (!buffer && buffer_size)
        || (folder != FORGEEFX_FOLDER_NAM && folder != FORGEEFX_FOLDER_IR))
        return FORGEEFX_HOST_INVALID_ARGUMENT;
    if (!hostServices)
        return FORGEEFX_HOST_UNAVAILABLE;
    return hostServices->get_folder(hostServices->context, folder, buffer, buffer_size, required_size);
}
