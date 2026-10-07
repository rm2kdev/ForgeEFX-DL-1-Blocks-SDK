# Developing blocks for ForgeEFX DL-1

New to block development? Follow [Getting Started](Getting%20Started.md) for a
guided first build, a parameter exercise, and explanations of the DSP interface.

This guide takes you from the bundled gain template to a standalone `.fxblock`
package. SDK 1.0.0 exposes ABI v1. The DL-1 host loads compatible external
blocks; the SDK does not include the host application.

Visit [www.forgeefx.com](https://www.forgeefx.com/) for ForgeEFX product information.

## 1. Install the build tools

- CMake 3.22 or newer and Python 3.10 or newer, available on your command path.
- Windows: Visual Studio C++ build tools, the Windows SDK, and an x64 developer
  terminal. Windows ABI v1 packages target x64.
- macOS: Xcode command-line tools and CMake/Python. Build for Apple Silicon,
  Intel, or both. The host and block must have compatible architectures.

The source uses C11 for DSP and C++20 for the validator. There are no downloaded
runtime dependencies. The CMake helper can name Linux packages for local work,
but this repository does not establish Linux DL-1 host support.

## 2. Build and test the included example

From the repository root:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
```

CTest loads the actual example modules with the SDK validator, checks the gain
and delay algorithms, tests host-version metadata, and checks Mac architecture selection.
Architecture-selection tests do not run a Mac binary.

For an explicit Visual Studio x64 build, add `-A x64` to the configure command
and use a fresh build directory. Do not use `-A` with Ninja. For single-config
generators such as Ninja, `-DCMAKE_BUILD_TYPE=Release` selects optimization;
Visual Studio and other multi-config generators use `--config Release`.

The Windows output is:

```text
build/native/dist/yourcompany.youreffect.fxblock/
  manifest.json
  bin/windows-x64/block.dll
```

The developer and effect IDs are deliberately buildable placeholders. The gain
example defaults to unity gain, permits 0..200 percent gain, and saturates to
the Q16 full-scale range. It uses the host's default parameter editor.

The combined build also creates `build/native/dist/yourcompany.delay.fxblock/`.
`examples/delay` demonstrates a fixed circular delay buffer and a basic custom
editor with Time, Feedback and Mix controls. See the root README for its ranges,
standalone build command and intentional limitations. These examples are
unpublished templates with placeholder identities.

## 3. Give your block its own identity

Copy `examples/gain` into a new directory. Choose a stable namespace you control,
then replace the following values before sharing or distributing a block:

| Source | Template value | Meaning |
| --- | --- | --- |
| `parameters.json` -> `id` | `yourcompany.youreffect` | Permanent namespaced effect identity |
| `CMakeLists.txt` -> `DEVELOPER` | `yourcompany` | Developer identity |
| `parameters.json` -> `name` | `SDK GAIN` | Display name |
| `parameters.json` -> `package_version` | `1.0.0` | Version of your block package |

Keep the effect ID stable across compatible updates. Use a namespaced ID with
letters, digits, dots, underscores or hyphens. Unnamespaced catalog IDs and
official legacy indexes are reserved; omit `index` for a new effect (it becomes
-1). Do not use a `forgeefx.*` identity for your own block.

The `gain_process` and `gain_reset` names are ordinary C functions, not publisher
identities. You may rename them if you update their source definitions and the
matching manifest fields together. Keep the SDK export `forgeefx_get_block_api`
unchanged. Internal CMake target names such as `example_gain` are not effect IDs.

The generator reads optional category display metadata from `category.json` in
the effect folder's parent. The parent directory name becomes `category_id`.
This repository provides `examples/category.json`. In a collection, use a layout
such as `effects/drive/my_effect/` with `effects/drive/category.json` containing
`name`, `short_name`, and `order` for that category.

## 4. Build outside this repository

The copied effect's CMake file accepts `FORGEEFX_SDK_DIR`. Point it to the `sdk`
subdirectory of this checkout, or to an independently copied SDK directory.
It does not require a Blocks or host checkout.

```sh
cmake -S my-effect -B build/my-effect -DCMAKE_BUILD_TYPE=Release -DFORGEEFX_SDK_DIR="/absolute/path/to/ForgeEFX-DL-1-Blocks-SDK/sdk"
cmake --build build/my-effect --config Release --parallel
```

On Windows replace the SDK path with your own drive path, retaining quotes if
it contains spaces. The resulting package is under `build/my-effect/dist/`.

The minimal effect project is:

```cmake
cmake_minimum_required(VERSION 3.22)
project(MyEffect LANGUAGES C)
set(FORGEEFX_SDK_DIR "" CACHE PATH "Path to the ForgeEFX SDK")
include("${FORGEEFX_SDK_DIR}/cmake/ForgeEFXBlock.cmake")
forgeefx_add_block(my_effect "${CMAKE_CURRENT_SOURCE_DIR}" DEVELOPER yourcompany)
```

The helper compiles `dsp.c`, its generated descriptor, and optional `ui.c` with
the rendering bridge. It does not glob other implementation files. Add extra C
sources explicitly with `target_sources(my_effect PRIVATE helper.c)`; enable CXX
in `project()` and preserve C linkage if adding C++ implementations. Use
`INCLUDE_DIRECTORIES` for shared headers and `OUTPUT_DIRECTORY` to override the
package destination. Do not link against host or catalog code.

To add a block to this repository's combined build, add its directory with
`add_subdirectory(...)` after `add_subdirectory(sdk)` in the root CMake file.
This makes the existing validator available when the block target is created,
so `forgeefx_add_block` registers its compiled-module CTest check.

## 5. Implement the DSP contract

`sdk/include/forgeefx_block.h` defines the authoritative ABI. The basic callbacks
are:

```c
int gain_process(int sample, int *params, int *state);
void gain_reset(int *state);
```

- `sample` is a signed Q16 sample at 48,000 Hz; 65,536 represents 1.0.
  Use widened arithmetic for products and a defined output-range policy.
- `params` contains 64 integer slots. Your declared parameters occupy the first
  slots in manifest order. Treat their values as host-owned inputs.
- `state_words` reserves that many 32-bit words per channel. The host initially
  zeroes state; `reset` must restore deterministic behavior after prior processing.
  Declare enough space and keep all mutable instance state there.
- The host supplies independent channel states. Optional `stereo_process` has
  the exact signature in the ABI header; update both output pointers on every call.
- Never allocate, block, lock, log, access files, load models, or draw inside DSP
  or reset. Do not use mutable globals for instance state. Do not let exceptions
  cross the C ABI. The host owns routing and sample-rate conversion.

`state_alignment` is 4 or 8 bytes in ABI v1 (default 8). Do not require stronger
alignment or assume packed structs. The SDK does not expose file/model services
or general custom-state serialization. Keep ABI layouts and calling conventions
unchanged when adding your own effect.

### Analog circuit voltage convention

For analog circuit models, first read the
[voltage convention](sdk/README.md#analog-circuit-blocks). Include the bundled
`expansion_analog.h`, convert Q16 input with `axInput(sample) * AX_IN_VOLTS`,
process the circuit in physical volts, and return
`axOutput(output_volts / AX_OUT_VOLTS)`. The calibrated full-scale values are
5.62 V peak in and 4.04 V peak out, not RMS. Convert only at the circuit's jacks;
preserve component gain, rails and branch mixing in volts. Equal jack voltage
is about +2.87 dB digitally, not digital unity. Digital effects and software
bypass retain normalized Q16 processing. Existing examples need no rescaling.

For a migrated analog block, compare peak levels and CPU cost before/after,
preserve saved parameter definitions, and document possible changes in drive
and level. Check circuit-based plots and meter conversions in the actual host.
The SDK tests verify the conversion convention; they do not establish hardware
accuracy for your circuit.

## 6. Define parameters and compatibility

Edit `parameters.json`, using `sdk/parameters.schema.json` as the reference.
The source example includes explicit package and host-version metadata.

- Declare 1..64 parameters with unique `name` keys. `name` is the persistent key;
  optional `display_name` changes the visible label without changing that key.
- `min`, `max`, and `default` are integers. Bounds must be within +/-1,000,000;
  range width must be at most 1,000,000. Defaults must lie inside the range.
- Optional `labels` describes every choice from min through max, inclusive.
  `unit` supplies a display suffix; `format` supports `number`, `knob`, `signed`.
- Preserve released keys, parameter ordering, ranges and meaning. These values
  are part of the host's preset/automation compatibility contract.
- `tempo_param`, `sync_param`, and `trails_param` are zero-based parameter indexes,
  or -1 when unused. `tail_seconds` is 0 for no tail, positive for a finite tail,
  or -1 for an indefinite tail; the generator accepts -1..3600.
- `minimum_host_version` is inclusive and `maximum_host_version` exclusive.
  The template's range is `0.1.0 <= host < 0.2.0`. Use strict `major.minor.patch`
  versions and only widen compatibility after testing the intended hosts.

The generated package `manifest.json` and compiled descriptor come from the
same source metadata. Never hand-edit either generated file. Reconfigure/rebuild
after editing the source manifest. Keep package and binary together.

Host-version compatibility, SDK ABI version, and package version are separate.

## 7. Add an optional editor

Omit `ui.c` to use the host's default editor. For a custom editor, add `ui.c`, set
`render` in the source manifest, and implement a function taking `const BlockUI *`.
For example, with `"render": "my_effect_render"`:

```c
#include "block_ui.h"

void my_effect_render(const BlockUI *ui)
{
    block_ui_begin(ui);
    block_ui_text(8, 20, "MY EFFECT");
    block_ui_control_knob(ui, 0, 80, 36);
    block_ui_end(ui);
}
```

Draw only between begin/end. Author on the logical 160x80 canvas; use the SDK's
layout constants and helpers for the host-controlled chrome and content area.
Rendering runs on the message thread. UI and drawing-service pointers expire at
the end of each render call; never retain them or use drawing helpers in DSP.
The SDK bridge binds and restores the service table automatically.

Inspect screenshots in the actual host at relevant pages, parameter extremes and
bypass states. The standalone validator uses stub drawing services, so it can
detect missing rendering calls but cannot prove correct artwork or layout.

## 8. Validate a standalone package

Build the validator independently if you are not using the combined root build:

```sh
cmake -S sdk -B build/validator -DCMAKE_BUILD_TYPE=Release
cmake --build build/validator --config Release --parallel
```

For Visual Studio builds on Windows, run from PowerShell:

```powershell
& ./build/validator/Release/forgeefx_block_validator.exe ./build/my-effect/dist/yourcompany.youreffect.fxblock/bin/windows-x64/block.dll
```

For Ninja on Windows, the executable is directly in `build/validator/`. For
macOS single-config builds, use:

```sh
./build/validator/forgeefx_block_validator ./build/my-effect/dist/yourcompany.youreffect.fxblock/bin/macos-arm64/block.dylib
```

Replace placeholder IDs and platform directories with your actual output. The
validator checks ABI negotiation, descriptor metadata, state independence,
deterministic reset, parameter extremes and optional rendering calls. It executes
native module code and is not a sandbox or an audio-quality test.

Test silence, clipping/headroom, rapid parameter changes, repeated reset,
multiple instances, mono/stereo behavior, and any tails. Then test loading,
presets, automation, bypass and editor behavior in the supported DL-1 hosts/DAWs.
Record listening results separately from automated checks in `VERIFICATION.md`.

## 9. Build for Mac and distribute

On a Mac, a universal example build is:

```sh
cmake -S . -B build/mac -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
cmake --build build/mac --config Release --parallel
ctest --test-dir build/mac -C Release --output-on-failure
```

Its binary is `bin/macos-universal/block.dylib`. Single-architecture outputs use
`macos-arm64` or `macos-x64`. Building on Windows does not produce Mac binaries;
test on the intended Mac architectures and hosts and complete required signing/
notarization for your distribution workflow.

Before distributing, replace all template identity values, bump your package
version appropriately, inspect generated metadata, and validate the actual binary.
Ship the entire `.fxblock` directory. If combining platform binaries into one
package, use identical source, identity, package version and metadata for each.

Close DL-1 and any DAW hosting it before replacing a loaded library. Copy the
complete package into a Blocks directory scanned by your host installation and
restart the host. A missing block commonly means an unscanned directory, a nested
or incomplete package, an architecture mismatch, an incompatible host range,
or a duplicate ID. Verify those against the actual generated metadata and host
configuration before changing DSP code.
