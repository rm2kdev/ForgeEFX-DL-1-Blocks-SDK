#include "forgeefx_host_services.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static void require(bool condition, const char *message)
{
    if (!condition) throw std::runtime_error(message);
}

struct Folders {
    std::array<std::string, 2> paths { "C:/Users/test/Models", "C:/Users/test/IRs" };
    int32_t status = FORGEEFX_HOST_OK;
    unsigned calls = 0;
};

static int32_t getFolder(void *context, uint32_t folder, char *buffer,
                         uint32_t size, uint32_t *required)
{
    auto &folders = *static_cast<Folders *>(context);
    ++folders.calls;
    if (folders.status != FORGEEFX_HOST_OK) return folders.status;
    const auto &path = folders.paths.at(folder - 1);
    *required = static_cast<uint32_t>(path.size() + 1);
    if (size < *required) return FORGEEFX_HOST_BUFFER_TOO_SMALL;
    std::memcpy(buffer, path.c_str(), *required);
    return FORGEEFX_HOST_OK;
}

int main(int argc, char **argv)
{
    try {
        require(argc == 3, "Usage: host_services_tests <module> <current|legacy>");
#ifdef _WIN32
        auto module = LoadLibraryA(argv[1]);
        const auto symbol = [module](const char *name) { return GetProcAddress(module, name); };
#else
        auto module = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
        const auto symbol = [module](const char *name) { return dlsym(module, name); };
#endif
        require(module != nullptr, "Could not load fixture module");
        auto entry = reinterpret_cast<ForgeEFXGetBlockApi>(symbol("forgeefx_get_block_api"));
        require(entry && entry(FORGEEFX_BLOCK_ABI_VERSION), "Existing ABI v1 entry unavailable");
        require(!entry(FORGEEFX_BLOCK_ABI_VERSION)->render, "Fixture must work without an editor");
        auto bind = reinterpret_cast<ForgeEFXSetHostServices>(symbol("forgeefx_set_host_services"));
        if (std::string(argv[2]) == "legacy") {
            require(!bind, "Legacy module unexpectedly exports host services");
        } else {
            using Query = int32_t (*)(uint32_t, char *, uint32_t, uint32_t *);
            auto query = reinterpret_cast<Query>(symbol("forgeefx_test_query_folder"));
            require(bind && query, "Current module lacks host-services bridge");
            std::array<char, 256> buffer {};
            uint32_t required = 999;
            const auto read = [&] { return query(FORGEEFX_FOLDER_NAM, buffer.data(), uint32_t(buffer.size()), &required); };
            require(read() == FORGEEFX_HOST_UNAVAILABLE && required == 0, "Older host fallback failed");
            Folders folders;
            ForgeEFXHostServices services { FORGEEFX_HOST_SERVICES_ABI_VERSION, sizeof(ForgeEFXHostServices), &folders, getFolder };
            struct Prefix { uint32_t abi_version; uint32_t struct_size; } prefix { 1, sizeof(Prefix) };
            bind(reinterpret_cast<const ForgeEFXHostServices *>(&prefix));
            require(read() == FORGEEFX_HOST_UNAVAILABLE, "Short struct accepted");
            services.abi_version = 2;
            bind(&services);
            require(read() == FORGEEFX_HOST_UNAVAILABLE, "Unknown service ABI accepted");
            services.abi_version = 1;
            services.get_folder = nullptr;
            bind(&services);
            require(read() == FORGEEFX_HOST_UNAVAILABLE, "Missing callback accepted");
            services.get_folder = getFolder;
            bind(&services);
            require(read() == FORGEEFX_HOST_OK && std::string(buffer.data()) == folders.paths[0], "NAM folder mismatch");
            require(query(FORGEEFX_FOLDER_IR, buffer.data(), uint32_t(buffer.size()), &required) == FORGEEFX_HOST_OK
                && std::string(buffer.data()) == folders.paths[1], "IR folder mismatch");
            require(query(FORGEEFX_FOLDER_NAM, nullptr, 0, &required) == FORGEEFX_HOST_BUFFER_TOO_SMALL
                && required == folders.paths[0].size() + 1, "Size query excludes NUL or has wrong status");
            buffer[0] = 'x';
            require(query(FORGEEFX_FOLDER_NAM, buffer.data(), 1, &required) == FORGEEFX_HOST_BUFFER_TOO_SMALL
                && buffer[0] == '\0', "Small buffer returned truncated path");
            require(query(FORGEEFX_FOLDER_NAM, buffer.data(), required, &required) == FORGEEFX_HOST_OK,
                "Exact-sized buffer rejected");
            const auto previousCalls = folders.calls;
            require(query(0, buffer.data(), uint32_t(buffer.size()), &required) == FORGEEFX_HOST_INVALID_ARGUMENT
                && required == 0 && buffer[0] == '\0', "Unknown folder accepted");
            require(query(FORGEEFX_FOLDER_NAM, nullptr, 1, &required) == FORGEEFX_HOST_INVALID_ARGUMENT,
                "Null buffer with positive capacity accepted");
            require(query(FORGEEFX_FOLDER_NAM, buffer.data(), uint32_t(buffer.size()), nullptr) == FORGEEFX_HOST_INVALID_ARGUMENT,
                "Null size output accepted");
            require(folders.calls == previousCalls, "Invalid arguments reached host");
            folders.paths[0] = "C:/Users/\xC3\xA9/\xE6\xA8\xA1\xE5\x9E\x8B";
            require(read() == FORGEEFX_HOST_OK && std::string(buffer.data()) == folders.paths[0]
                && required == folders.paths[0].size() + 1, "Updated UTF-8 folder not visible");
            folders.status = FORGEEFX_HOST_ERROR;
            require(read() == FORGEEFX_HOST_ERROR && required == 0 && buffer[0] == '\0', "Host error not propagated");
            folders.status = FORGEEFX_HOST_UNAVAILABLE;
            require(read() == FORGEEFX_HOST_UNAVAILABLE, "Unavailable folder not propagated");
            bind(nullptr);
            require(read() == FORGEEFX_HOST_UNAVAILABLE, "Null binding retained services");
        }
#ifdef _WIN32
        FreeLibrary(module);
#else
        dlclose(module);
#endif
        std::cout << "Host services compatibility passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
