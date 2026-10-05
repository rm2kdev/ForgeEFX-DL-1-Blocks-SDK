# Verification

## Responsive UI demo - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake 4.4.3/Ninja, Release, Python 3.12:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
cmake -S examples/responsive_ui -B build/standalone -DCMAKE_BUILD_TYPE=Release -DFORGEEFX_SDK_DIR=<absolute-sdk-path>
cmake --build build/standalone --config Release --parallel
```

Configuration also supplied local compiler, generator, and Python selections.
Root CTest: **7/7 passed**. The UI contract test exercises 108 combinations of
capture availability, enabled/loading state, gain extremes/default, and recent,
expired, or absent edits. It checks lifecycle pairing, logical drawing bounds,
trace containment, silence fallbacks, focus/edit feedback, and unchanged inputs.
These are service-substitute checks, not pixel or host-layout verification.
The demo also runs the existing gain behavior tests against its own DSP source.
Compiled-module validation checks reset, silence after an impulse, parameter
extremes, independent channel/instance state, and the render bridge.

```powershell
& ./build/native/sdk/forgeefx_block_validator.exe ./build/standalone/dist/yourcompany.responsive_ui_demo.fxblock/bin/windows-x64/block.dll
```

The standalone DLL passed. Generated metadata was inspected: developer
`yourcompany`, effect `yourcompany.responsive_ui_demo`, package `1.0.0`, host
range `[0.1.0, 0.2.0)`, ABI v1, and Windows x64 binary. Identity values remain
explicit unpublished example placeholders, not a distribution release.

The initial combined Ninja build exposed inconsistent SDK path normalization
between examples. Supplying the canonical SDK path in the root CMake project
resolved duplicate generation dependencies without changing SDK internals.
Only external compiler/build tools were reused; host/catalog source and assets
are not dependencies and were not copied into the example.

### Windows host, listening, and DAW checks

Inspected a running DL-1 host screenshot as a reference for its monochrome LCD
and host-controlled header/footer. The new demo was **not** loaded into that
running session. Demo artwork screenshots, physical display scaling, audio
listening, presets/automation, and DAW checks remain unrun. The automated checks
do not establish visual correctness or host integration. Follow the example's
README for the outstanding host checks.

### Mac host checks

No Mac compilation, runtime, artwork, or DAW checks were performed. The existing
Mac platform-selection tests passed on Windows only.

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
