# Verification

## Host content folder extension - 8 October 2026

### Automated checks

Added an optional, versioned host-services export and a module-local NAM/IR
folder helper without changing block/render ABI v1 layouts. The CMake helper
includes the bridge with and without custom UI. Documentation explains runtime
capability checks, caller-owned UTF-8 buffers, lifetimes and worker/UI use.

Windows x64, MSVC 19.51, Ninja, Release. Commands in `.worktrees/folders-sdk`,
with external compiler/Windows SDK tools and Python selected in the shell:

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python>
cmake --build build/native --config Release --parallel 4 --target forgeefx_block_validator host_services_tests host_services_fixture host_services_legacy_fixture example_gain example_delay
ctest --test-dir build/native -C Release --output-on-failure -R "^(sdk_host_services.*|package_example_gain|package_example_delay)$"
cmake -S sdk -B build/standalone-sdk -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/standalone-sdk --config Release --parallel 4
ctest --test-dir build/standalone-sdk -C Release --output-on-failure -R "^sdk_host_services.*$"
git diff --check
```

Root focused checks passed **6/6**. Independent SDK checks passed **4/4**.
The coordinator merged the branch in a separate integration worktree, repeated
the six focused checks successfully, fast-forwarded `main`, and rebuilt the
validator, extension fixtures, and all four example packages there.
Tests load actual current/legacy DLL fixtures, preserve the legacy entry point,
query without an editor, check absent binding, null/short/incompatible tables,
missing callbacks, invalid arguments, exact/small buffers, UTF-8 byte counts,
live folder edits and host error propagation. Validators accept both fixture
generations and representative gain/default-editor and delay/custom-editor
packages. Generated example identities and host bounds remain unchanged.

Extracted the guide's complete C folder-copy example into an ignored build file;
`cl /c /std:c11 /I sdk/include` compiled it successfully. Shared extension files
match the companion Blocks project; its customized generator was not replaced.
The first CTest command resolved a Python wrapper without its package path;
calling the CTest executable beside CMake corrected the environment. Diff check
passed. No generated package files, dependencies or local tool paths were committed.

### Listening, Windows host, macOS host and DAW checks

Not run in this SDK change. Synthetic validator paths do not establish real
folder selection, model loading or audio quality. Host integration is validated
in the host repository. No DSP, model or IR loader was added or changed here.

## Standard analog voltage support - 7 October 2026

### Automated checks

Imported the public SDK's independent `expansion_analog.h` unchanged from
Blocks revision `273879ec9166cb751e229511f1e40879ae86af4d` and documented the
5.62 V peak input / 4.04 V peak output convention. ABI declarations, manifests,
parameter definitions and example DSP/editor sources are unchanged.

Windows x64, MSVC 19.51, CMake 4.4.4/Ninja 1.13.2, Python 3.12.14, Windows SDK
10.0.26100.7175. Initial configure attempts exposed missing tool paths and
Windows SDK components. CMake, Ninja and the Microsoft Windows SDK NuGet
packages were installed in an external user cache, and the compiler environment
was corrected before the successful runs. No host/catalog build dependency or
tool binaries were added to this repository.

Commands in `.worktrees/voltage-builder`, with external tool paths selected in
the shell (the Python placeholder below represents that local selection):

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python-executable>
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
cl /nologo /TP /std:c++20 /EHsc /I sdk/include tests/AnalogVoltageTests.c /Fo:build/native/analog_cpp.obj /Fe:build/native/analog_cpp.exe
./build/native/analog_cpp.exe
git diff --check
```

Root build succeeded; **12/12 tests passed**. The new test checks positive and
negative measured jack levels, physical unity's +2.867 dB digital ratio, silence,
integer extremes, clipping and non-finite output. It also passed when compiled
as C++20. Existing tests cover reset, state independence, parameter extremes,
channel isolation, digital example behavior and all four compiled example DLLs.
The imported header's SHA-256 matched the reference copy. Diff check passed.

Copied only `sdk/` to `build/sdk-copy/` and built gain against that copy:

```powershell
Copy-Item -LiteralPath sdk -Destination build/sdk-copy -Recurse
cmake -S examples/gain -B build/standalone -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python-executable> "-DFORGEEFX_SDK_DIR=<worktree>/build/sdk-copy"
cmake --build build/standalone --config Release --parallel
./build/native/sdk/forgeefx_block_validator.exe ./build/standalone/dist/yourcompany.youreffect.fxblock/bin/windows-x64/block.dll
```

Standalone build and compiled-DLL validator passed. Inspected root packages
`yourcompany.youreffect`, `yourcompany.delay`, `yourcompany.phaser`, and
`yourcompany.responsive_ui_demo`: developer `yourcompany`, package version
`1.0.0`, ABI v1, host range `[0.1.0, 0.2.0)`, 48 kHz and existing
`bin/windows-x64/block.dll` binaries. Standalone gain retains the same metadata.
These are unpublished examples with intentional placeholder identities, not
distribution-ready effects.

After merging into `.worktrees/voltage-integration`, repeated the root
configure/build/CTest commands in a fresh build directory: build succeeded and
**12/12 tests passed**. Verified all four DLLs have x64 PE machine type and
asserted the package metadata above. All 35 local links in the updated public
guides and provenance document resolve. Integration diff check passed.

### Listening, Windows host, macOS host and DAW checks

Not run. No example DSP or artwork changed. No new hardware measurements or
circuit-accuracy claims are made. Mac architecture-selection tests do not run
Mac binaries. The new conversion tests do not validate every unrelated primitive
in the shared header.

## README agent introduction - 5 October 2026

### Automated checks

Added an introduction to creating bespoke effects with an LLM coding agent,
links to the repository instructions, and a sample prompt. Documentation only.
Windows x64, MSVC 19.51, CMake/Ninja, Release, Python 3.12. Commands run in
`.worktrees/llm-blocks-writer` with external build tools selected in the shell:

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python-executable>
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
git diff --check
```

The root build succeeded and **11/11 tests passed**, including validation of
all four compiled example DLLs. All 17 README local links resolve and the diff
check passed. Inspected generated manifests: the four example identities retain
developer `yourcompany`, package version `1.0.0`, ABI v1 and host range
`[0.1.0, 0.2.0)`. The build produced their `bin/windows-x64/block.dll` binaries.
These remain unpublished templates with intentional placeholder identities.

Repeated the root configure/build/CTest commands in the fresh
`.worktrees/llm-blocks-integration` worktree: build succeeded and **11/11 tests
passed**. The integration diff check also passed.

### Listening, Windows host, macOS host and DAW checks

Not run for this documentation change. No DSP, editor or SDK behavior changed.

## README example screenshots - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake/Ninja, Release, Python 3.12. Commands run in
the screenshot task worktree, with local compiler, Ninja and Python selections:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
```

The root build succeeded and **11/11 tests passed**, including compiled-module
validation for all four examples. Inspected the three captured packages:
`yourcompany.delay`, `yourcompany.phaser`, and `yourcompany.responsive_ui_demo`;
each uses developer `yourcompany`, package version `1.0.0`, ABI v1, host range
`[0.1.0, 0.2.0)`, and `bin/windows-x64/block.dll`. These remain unpublished
example identities. No DSP, SDK or package metadata was changed.

Merged with the current documentation in a fresh integration worktree and
repeated the root configure/build/CTest commands: build succeeded and **11/11
tests passed**. All README local links resolve. The three captures retain their
native JPEG encoding at 1211 x 655 pixels (about 263 KiB combined).
`git diff --check` passed.

### Windows host visual checks

Launched the existing Windows DL-1 standalone executable with a process-local
`FORGEEFX_BLOCKS_PATH` pointing at the task build's complete example packages.
Loaded Delay, Phaser and Responsive UI from the Examples menu and inspected
their editors at defaults. Saved unaltered application-window captures in
`docs/screenshots/` and embedded them in the root README with source links and
descriptive alt text. Delay shows 300 ms / 30% / 30%; Phaser shows rate 5 in
0.1 Hz units (0.5 Hz) and depth 75%; Responsive UI shows 100% gain and a flat
output trace with no input signal. Captures establish loading and appearance
at these settings, not audio routing or listening quality.

### Listening, macOS host and DAW checks

Not run. Parameter extremes, bypass, presets, automation and live waveform
response were not checked in the host for this documentation change.

## Getting started guide - 5 October 2026

### Automated checks

Documentation only: added a beginner walkthrough and links from the root README,
development guide, and SDK README. The guide targets approximately 80%
ASD-STE100-style prose. This is an editorial target, not formal certification
or a measured dictionary-compliance result.

Windows x64, MSVC 19.51, CMake 4.4.3/Ninja, Release, Python 3.12.14.
Commands run in `.worktrees/getting-started-writer` with external compiler and
Windows SDK tools selected in the shell:

```sh
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python-executable>
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --output-on-failure
```

The root build succeeded and **11/11 CTest checks passed**. These include the
actual example binaries and existing DSP tests. No SDK or DSP source changed.

Rehearsed the Windows copy-and-edit procedure under the ignored
`build/walkthrough` folder. Copied only the public SDK and gain template into a
temporary SDK layout, copied the template to its sibling `my-first-block`, and
changed the source gain default from 100 to 50. From that temporary SDK root:

```powershell
$sdkRoot = (Get-Location).Path
cmake -S ../my-first-block -B build/first-block -G Ninja -DCMAKE_BUILD_TYPE=Release "-DFORGEEFX_SDK_DIR=$sdkRoot/sdk" -DPython3_EXECUTABLE=<python-executable>
cmake --build build/first-block --config Release --parallel
& <task-worktree>/build/native/sdk/forgeefx_block_validator.exe ./build/first-block/dist/yourcompany.youreffect.fxblock/bin/windows-x64/block.dll
```

The standalone build and actual DLL validation passed. Inspected the generated
manifest and descriptor: gain default 50, effect `yourcompany.youreffect`,
developer `yourcompany`, package `1.0.0`, host range `[0.1.0, 0.2.0)`, ABI v1,
48 kHz, and `bin/windows-x64/block.dll`. Placeholders remain intentional for the
unpublished exercise. No distribution package was committed.
An initial rehearsal setup command resolved a relative path against the wrong
directory; using PowerShell `Resolve-Path` corrected the temporary harness.
The documented copy and CMake commands required no correction.

A one-off documentation check passed for 23 local links, including the editor
section anchor, and balanced code fences in all four guide/reference files.
Reviewed the guide against the source schema, gain implementation, CMake helper,
and existing development guide. A prose-length check found no non-table prose
lines over 25 whitespace-separated words; this is not a full STE language audit.
`git diff --check` passed.

Merged the guide in `.worktrees/getting-started-integration` and repeated the
root configure, build, and CTest commands above in a fresh build directory.
The integration build succeeded and **11/11 tests passed**. The integration
diff check also passed.

### Listening, Windows host, macOS host and DAW checks

Not run for this documentation change. No Mac compilation, Visual Studio
generator build, listening, host loading, artwork inspection, or DAW tests were
performed. Mac platform-selection tests ran on Windows only. The guide separates
compiled-module validation from the host and listening checks still required
for a developer's own block.

## Public documentation refresh - 5 October 2026

### Automated checks

Windows x64, MSVC 19.51, CMake/Ninja, Release, Python 3.12:

- Added website links to the root README, development guide and SDK README,
  and removed internal product-policy references from public repository files.
- `git diff --check`: passed.
- Configured with `cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release`
  plus Ninja and local Python selection; built with
  `cmake --build build/native --config Release --parallel`.
  An initial configure failed because the local Windows SDK library path was
  incorrect; correcting the build environment resolved it without source changes.
- `ctest --test-dir build/native -C Release --output-on-failure`: **4/4 passed**
  in the task worktree, including the compiled-module validator.
- Repeated the same configure, build and CTest commands in a fresh integration
  worktree: build succeeded and **4/4 passed**.
- Inspected generated package metadata and binary: effect ID
  `yourcompany.youreffect`, developer ID `yourcompany`, host range
  `[0.1.0, 0.2.0)`, and `bin/windows-x64/block.dll`.

### Listening, Windows host, macOS host and DAW checks

Not run for this documentation change. DSP and SDK runtime behavior are unchanged.

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
The fresh integration worktree also built successfully and passed **7/7** tests.

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

Merged the delay work with the concurrently landed public documentation and
responsive UI demo in `.worktrees/delay-integration`, preserving their changes.
The root configure/build commands above succeeded there and CTest passed **9/9**
checks. Integration reused the canonical SDK path setup already added by the
responsive UI demo. `git diff --check` passed. No SDK implementation changes
were needed.

The minimal phaser demo subsequently landed on `main`; merged it into the same
integration worktree, preserved both examples and verification histories, and
reran root configure/build/CTest: **11/11 passed**.

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
- Repeated the root configure/build/CTest commands in a fresh integration
  worktree alongside the responsive UI example: **9/9 tests passed**.
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
Independent source review found no material issues in the editor layout or
render lifecycle; this is not a visual approval of the unobserved editor.

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
