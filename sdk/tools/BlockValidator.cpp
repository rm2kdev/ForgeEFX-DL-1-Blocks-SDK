#include "forgeefx_block.h"
#include "forgeefx_host_services.h"
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static int drawCalls;
#define FORGEEFX_RENDER_SERVICE(result, name, args) static result stub_##name args { ++drawCalls; return result(); }
#include "forgeefx_render_services.inc"
#undef FORGEEFX_RENDER_SERVICE
static int clampValue(int value, int low, int high) { return std::clamp(value,low,high); }
static void formatValue(char* output, int, int, int) { std::strcpy(output,"0"); }
static void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

static int32_t getFolder(void*, uint32_t folder, char* buffer, uint32_t size, uint32_t* required)
{
    if (buffer && size) buffer[0] = '\0';
    if (required) *required = 0;
    if (!required || (!buffer && size) || (folder != FORGEEFX_FOLDER_NAM && folder != FORGEEFX_FOLDER_IR))
        return FORGEEFX_HOST_INVALID_ARGUMENT;
    // Validator exposes synthetic paths; it never creates or loads content.
#ifdef _WIN32
    const char* path = folder == FORGEEFX_FOLDER_NAM ? "C:/ForgeEFX-validator/NAM" : "C:/ForgeEFX-validator/IR";
#else
    const char* path = folder == FORGEEFX_FOLDER_NAM ? "/ForgeEFX-validator/NAM" : "/ForgeEFX-validator/IR";
#endif
    *required = static_cast<uint32_t>(std::strlen(path) + 1);
    if (size < *required) return FORGEEFX_HOST_BUFFER_TOO_SMALL;
    std::memcpy(buffer, path, *required);
    return FORGEEFX_HOST_OK;
}

int main(int argc, char** argv)
{
    try {
        require(argc == 2, "Usage: forgeefx_block_validator <compiled-library>");
#ifdef _WIN32
        auto module = LoadLibraryA(argv[1]);
        auto entry = module ? reinterpret_cast<ForgeEFXGetBlockApi>(GetProcAddress(module, "forgeefx_get_block_api")) : nullptr;
#else
        auto module = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
        auto entry = module ? reinterpret_cast<ForgeEFXGetBlockApi>(dlsym(module, "forgeefx_get_block_api")) : nullptr;
#endif
        require(module && entry, "Library or ABI entry point unavailable");
        require(!entry(FORGEEFX_BLOCK_ABI_VERSION + 1), "Unsupported ABI accepted");
        auto api = entry(FORGEEFX_BLOCK_ABI_VERSION);
        require(api && api->abi_version == FORGEEFX_BLOCK_ABI_VERSION && api->struct_size >= sizeof(ForgeEFXBlockApi), "Invalid ABI descriptor");
        require(api->effect_id && api->name && api->developer_id && api->category_id, "Missing identity");
        require(api->sample_rate == 48000 && api->process && api->reset, "Invalid processing contract");
        require(api->parameter_count > 0 && api->parameter_count <= 64 && api->parameters, "Invalid parameter count");
        require(api->state_words > 0 && api->state_words <= 67108864 && api->state_alignment > 0 && api->state_alignment <= alignof(std::max_align_t), "Invalid state contract");
        std::array<int,64> params {};
        for (uint32_t i=0; i<api->parameter_count; ++i) {
            const auto& p = api->parameters[i];
            require(p.key && p.name && p.unit && p.minimum <= p.default_value && p.default_value <= p.maximum, "Invalid parameter metadata");
            if (p.choice_count) require(p.choices && p.choice_count == uint32_t(p.maximum-p.minimum+1), "Invalid choices");
            params[i] = p.default_value;
        }
        // The extension is optional. Legacy modules retain their ABI v1 path.
#ifdef _WIN32
        auto bind = reinterpret_cast<ForgeEFXSetHostServices>(GetProcAddress(module, "forgeefx_set_host_services"));
#else
        auto bind = reinterpret_cast<ForgeEFXSetHostServices>(dlsym(module, "forgeefx_set_host_services"));
#endif
        const ForgeEFXHostServices hostServices { FORGEEFX_HOST_SERVICES_ABI_VERSION,
            sizeof(ForgeEFXHostServices), nullptr, getFolder };
        if (bind) bind(&hostServices);
        std::vector<std::max_align_t> a((api->state_words * sizeof(int) + sizeof(std::max_align_t)-1) / sizeof(std::max_align_t));
        auto b = a;
        auto* sa = reinterpret_cast<int*>(a.data());
        auto* sb = reinterpret_cast<int*>(b.data());
        api->reset(sa); api->reset(sb);
        std::array<int,512> baseline {};
        for (int i=0;i<512;++i) baseline[i] = api->process(i == 0 ? 8192 : 0, params.data(), sa);
        for (int i=0;i<512;++i) require(baseline[i] == api->process(i == 0 ? 8192 : 0, params.data(), sb), "Instances share state");
        api->reset(sa);
        for (int i=0;i<512;++i) require(baseline[i] == api->process(i == 0 ? 8192 : 0, params.data(), sa), "Reset is not deterministic");
        for (int extreme=0;extreme<2;++extreme) {
            for (uint32_t i=0;i<api->parameter_count;++i) params[i] = extreme ? api->parameters[i].maximum : api->parameters[i].minimum;
            api->reset(sa); api->reset(sb);
            for (int i=0;i<512;++i) {
                if (api->stereo_process) { int left=0,right=0; api->stereo_process(i%2 ? 8192 : -8192, 4096, params.data(), sa, sb, &left, &right); }
                else api->process(i%2 ? 8192 : -8192, params.data(), sa);
            }
        }
        if (api->render) {
            ForgeEFXRenderServices services {};
            services.abi_version = FORGEEFX_BLOCK_ABI_VERSION;
            services.struct_size = sizeof(services);
#define FORGEEFX_RENDER_SERVICE(result, name, args) services.name = stub_##name;
#include "forgeefx_render_services.inc"
#undef FORGEEFX_RENDER_SERVICE
            services.block_ui_clamp = clampValue; services.block_ui_format = formatValue;
            for (uint32_t i=0;i<api->parameter_count;++i) params[i] = api->parameters[i].default_value;
            BlockUI ui {}; ui.params = params.data(); ui.enabled = 1; ui.edit_param = -1; ui.model_name = "";
            api->render(&ui, &services);
            require(drawCalls > 0, "Custom editor did not invoke host rendering services");
        }
        std::cout << api->effect_id << ": ABI, metadata, state independence, reset, parameter extremes and render passed\n";
#ifdef _WIN32
        FreeLibrary(module);
#else
        dlclose(module);
#endif
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
