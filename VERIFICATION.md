# Verification

## Delay demo - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake 4.4.3/Ninja, Release, Python 3.12.
Commands run from `.worktrees/delay-builder` after selecting the locally
installed compiler, CMake, Ninja, Python and Windows SDK in the shell:

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
cmake -S examples/delay -B build/delay -G Ninja -DCMAKE_BUILD_TYPE=Release -DFORGEEFX_SDK_DIR=<absolute-sdk-directory>
cmake --build build/delay --config Release --parallel
./build/native/sdk/forgeefx_block_validator.exe ./build/delay/dist/yourcompany.delay.fxblock/bin/windows-x64/block.dll
```

The configure commands also selected `Python3_EXECUTABLE` explicitly. All **6/6
CTest checks passed**, including the actual gain and delay DLLs. The standalone
delay DLL passed ABI, metadata, instance independence, reset, parameter extremes
and render-service validation. `delay_behavior` verifies exact impulse timing at
1, 300 and 1000 ms, positive/negative echoes, circular-buffer wrap, feedback
attenuation, default and endpoint mix behavior, full-scale saturation, separate
stereo lanes/instances, dirty and repeated reset, silence, rapid parameter
changes, state-allocation guards and decay within the declared 120-second tail.
Stereo uses the mono callback independently for each host-owned channel state.

Inspected generated package metadata and descriptor: `yourcompany.delay`,
developer `yourcompany`, version `1.0.0`, host range `[0.1.0, 0.2.0)`, ABI v1,
48 kHz, 48,001 state words per channel, three parameters, legacy index -1,
custom render callback, and `bin/windows-x64/block.dll`. Placeholder identities
are intentional for this unpublished SDK demo; it is not a distribution release.

The first combined Ninja build exposed two spellings of the SDK generator path
after adding a second example. Canonicalizing the shared SDK path in the root
CMake project removed the duplicate regeneration input; SDK internals and the
gain example remain unchanged. Build tools were reused externally; no host or
catalog code, dependencies, models or assets were copied or linked into the SDK.

### Windows host visual checks

Loaded the standalone package into the existing full Windows DL-1 application
using a process-local `FORGEEFX_BLOCKS_PATH`. The host discovered `SDK DELAY` in
Examples. Inspected actual editor screenshots at defaults (300 ms, 30%, 30%),
with each control selected, and at 1000 ms / 90% feedback / 69% mix. Labels,
knobs, formatted readouts and host controls were visible without artwork overlap.
Local screenshot evidence is in the task worktree's ignored `build/host-ui/`.
The running host executable changed during concurrent host development; minimum
values, 100% mix and bypass appearance were not conclusively checked. Validator
drawing stubs are not used as visual proof.

### Listening, DAW and macOS checks

No listening, DAW loading, preset round-trip, DAW automation, Mac compilation,
Mac execution or signing/notarization was performed. Delay-time changes are
intentionally unsmoothed and can click. Host screenshots establish editor
appearance only, not audio routing or audio quality.

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
