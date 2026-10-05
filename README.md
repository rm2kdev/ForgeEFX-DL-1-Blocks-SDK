# ForgeEFX DL-1 Blocks SDK

SDK 1.0.0 and a minimal gain effect for building external ForgeEFX DL-1 blocks.
Includes the ABI headers, drawing-service bridge, CMake package generator,
compiled-module validator and a working example. No host checkout, JUCE,
firmware, effect catalog or model downloads are needed.

Start with [Development Guide.md](Development%20Guide.md). Coding agents should
also read [AGENTS.md](AGENTS.md). The lower-level contract is in
[sdk/README.md](sdk/README.md) and [forgeefx_block.h](sdk/include/forgeefx_block.h).

## Quick start

Install CMake 3.22+, Python 3.10+, and a C11/C++20 compiler. On Windows use an
x64 Visual Studio developer terminal with the Windows SDK installed. On macOS
use Xcode command-line tools and install CMake and Python separately as needed.

```sh
git clone https://github.com/rm2kdev/ForgeEFX-DL-1-Blocks-SDK.git
cd ForgeEFX-DL-1-Blocks-SDK
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
```

The example package is `build/native/dist/yourcompany.youreffect.fxblock/`.
Its `yourcompany.youreffect` effect ID and `yourcompany` developer ID are valid,
buildable placeholders. Replace both before distributing your own block.

Copy the entire package folder into a Blocks location scanned by the full DL-1
host, then restart the host. Windows packages contain an x64 DLL; macOS packages
contain a dylib for the selected architecture. Build on each target platform.
Trial access belongs to the host's compiled ID allowlist; the example has no
default trial approval.

See [VERIFICATION.md](VERIFICATION.md) for tested platforms and outstanding
listening/host checks, and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for
the export's scope and source provenance.
