# Verification

## Minimal phaser demo - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake 4.4.3/Ninja, Python 3.12.14. Commands run
in `.worktrees/phaser-builder` with external compiler/Windows SDK tools on
the environment path and local CMake tool-selection options:

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
cmake -S examples/phaser -B build/phaser -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/phaser --config Release --parallel
./build/native/sdk/forgeefx_block_validator.exe ./build/phaser/dist/yourcompany.phaser.fxblock/bin/windows-x64/block.dll
```

- Root build succeeded; **6/6 CTest checks passed**. Phaser DSP checks include
  silence at rate/depth extremes through complete LFO cycles, impulse decay,
  deterministic reset, full-scale output bounds under rapid parameter changes,
  overload input, independent instances/channels, a cancellation notch near
  903 Hz at zero depth, and changed sweep behavior at different rates.
- Standalone build succeeded. Actual DLL passed ABI, metadata, independent
  state, reset, parameter extremes and render-service validation.
- Generated package inspected: `yourcompany.phaser`, developer `yourcompany`,
  package `1.0.0`, host bounds `[0.1.0, 0.2.0)`, five state words per channel,
  two parameters, 48 kHz and a Windows x64 DLL. Legacy index defaults to -1.
  This placeholder package remains an unpublished prototype, not a release.
- Initial configuration failed due to a local Windows SDK library path; corrected
  the build environment. The combined Ninja build then exposed duplicate SDK
  path spellings; the root now supplies one canonical SDK path to its examples.
  SDK internals are unchanged.

### Windows host and artwork checks

Launched the existing Windows host in a separate process pointing at the
standalone package directory. Inspected its initial screen, but desktop input
failed with stale screenshot IDs and `coordinate input geometry is unavailable`
before navigating to the phaser. Closed only this test process. Phaser loading,
editor appearance, extremes and bypass remain **unverified in the host**.
The validator's drawing stubs establish callback execution, not appearance.

### Listening, macOS and DAW checks

No listening, macOS build/runtime, DAW, preset round-trip or automation quality
checks were performed. The phaser has no parameter smoothing; abrupt automation
is bounded by saturation but may click. Mac platform-selection tests run on
Windows and do not establish Mac binary or host compatibility.

## Public SDK export - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake/Ninja, Release, Python 3.12:

- Configured and built the repository root in an isolated worktree with
  `cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release` plus local tool-selection
  options, then `cmake --build build/native --config Release --parallel 8`.
- `ctest --test-dir build/native -C Release --output-on-failure`: **4/4 passed**.
  These cover the actual compiled example module, gain behavior, four Python
  metadata/callback tests, and five Mac architecture-selection cases.
  A fresh build in the integration worktree also passed all four CTest checks.
- Copied only `sdk/` and `examples/gain/` into a separate directory containing
  spaces, with the effect folder named `my-effect`. Configured and built the
  copied effect using only `FORGEEFX_SDK_DIR` to find the copied SDK.
- Built the copied SDK validator separately and loaded the standalone Windows
  DLL successfully. The validator passed ABI, metadata, instance independence,
  deterministic reset and parameter-extreme checks. This example uses the
  default editor, so there was no custom editor to exercise.
- Inspected generated metadata: `effect_id` is `yourcompany.youreffect`,
  `developer_id` is `yourcompany`, and the host range is `[0.1.0, 0.2.0)`.
- Reproduced a generator failure for a no-editor effect in a hyphenated folder;
  the added regression failed before the fix and passed afterward. The fix
  avoids synthesizing an unused render symbol. Invalid explicitly supplied
  callback names still fail validation. The original gain descriptor was
  unchanged after the fix (the root rebuild required no recompilation).

The compiler, CMake and cached Windows SDK were reused as external build tools.
No host or effect-catalog source was included or linked. Build outputs, tools,
dependencies and local configuration are excluded from Git.

### Listening and Windows host checks

No listening, DL-1 application loading, DAW loading, preset/automation or artwork
checks were performed. Compiled-module validation does not establish audio quality
or full host integration. The example's DSP is unchanged from the exported gain
example; the SDK generator fix affects packaging only.

### Mac host checks

No Mac compilation, execution, signing/notarization or Intel/Apple Silicon host
checks were performed. The automated Mac checks test package-platform selection
logic on Windows, not Mac runtime compatibility.
