# Verification

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
