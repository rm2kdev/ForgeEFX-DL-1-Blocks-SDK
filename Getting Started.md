# Get started with DL-1 block development

Build a gain block first. Then change its controls and audio processing.
At the end of this guide, you will have a compiled block and a repeatable test procedure.

You need basic C knowledge and the ability to run terminal commands.
The SDK includes build helpers for blocks. It does not include the DL-1 host application.
You can build and validate a block before you install the host.

**Language approach:** This guide targets roughly 80% ASD-STE100-style writing.
Instructions use short sentences, direct verbs, and consistent terms.
Explanations retain necessary software terms and code identifiers.
This is an editorial target, not a measured compliance score.
See the [official ASD-STE100 explanation](https://www.asd-ste100.org/STE_faq.html) for the standard's writing rules and dictionary approach.

## 1. Understand what you will build

A **block** is an audio effect that the DL-1 host can load.
The **SDK** is the software development kit for these blocks.
**DSP** means digital signal processing. Your DSP code calculates the output audio samples.

A block package is a folder with the `.fxblock` extension.
It contains a **manifest** and a **binary**.
The manifest describes the block. The binary contains the compiled code for one operating system and processor architecture.

The build follows this sequence:

```text
dsp.c + parameters.json + CMakeLists.txt + optional ui.c
                         |
                    CMake build
                         |
             effect-id.fxblock/
               manifest.json
               bin/<platform>/block.dll or block.dylib
                         |
              Validator, then DL-1 host
```

Edit the source files. Let the SDK generate the package files.
Never edit generated `descriptor.c`, package `manifest.json`, or compiled binaries.

## 2. Install the required tools

Install these tools:

| Tool | Requirement | Purpose |
| --- | --- | --- |
| Git | Available in your terminal | Download the source and save changes |
| CMake | Version 3.22 or later | Configure the build and create packages |
| Python | Version 3.10 or later | Generate block metadata |
| C and C++ compiler | C11 and C++20 support | Compile blocks and the validator |

On Windows:

1. Install Visual Studio C++ build tools and the Windows SDK.
2. Open a Visual Studio developer PowerShell terminal for x64.

Windows ABI v1 blocks use x64 binaries.
The **ABI** is the binary interface between your block and the host.

On macOS:

1. Install the Xcode command-line tools.
2. Install CMake and Python if necessary.
3. Open Terminal.

The Mac host and block must have compatible processor architectures.
Use the [Development Guide](Development%20Guide.md) for universal Mac builds.
This SDK does not establish Linux host support.

Check the tools in your terminal:

```sh
git --version
cmake --version
python --version
```

On macOS, use `python3 --version` if the command is named `python3`.
CMake searches for Python 3 during configuration.

## 3. Build the supplied examples

1. Download the repository:

   ```sh
   git clone https://github.com/rm2kdev/ForgeEFX-DL-1-Blocks-SDK.git
   cd ForgeEFX-DL-1-Blocks-SDK
   ```

2. Configure the build:

   ```sh
   cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
   ```

   `-S` selects the source folder. `-B` selects the build folder.
   Configuration checks the tools and prepares the build files.

3. Compile the examples and validator:

   ```sh
   cmake --build build/native --config Release --parallel
   ```

4. Run the automated tests:

   ```sh
   ctest --test-dir build/native -C Release --output-on-failure
   ```

**Expected result:** The build succeeds and all tests pass.
The tests include checks against the compiled example binaries.

With a Visual Studio generator, add `-A x64` to the configure command if necessary.
Use a fresh build folder when you change the generator or architecture.
Do not use `-A` with Ninja.

The Windows gain package has this structure:

```text
build/native/dist/yourcompany.youreffect.fxblock/
  manifest.json
  bin/windows-x64/block.dll
```

Mac packages contain `block.dylib` in the selected Mac platform folder.
The template identities are placeholders for local development.

## 4. Copy the gain template

Create your effect in a separate folder beside the SDK checkout.
Keep the original template available for comparison.
The following commands start in the SDK repository root.
Use a new destination folder to avoid overwriting an existing project.

On Windows PowerShell:

```powershell
$sdkRoot = (Get-Location).Path
Copy-Item -LiteralPath ./examples/gain -Destination ../my-first-block -Recurse
```

On macOS:

```sh
sdk_root="$PWD"
cp -R examples/gain ../my-first-block
```

Keep this terminal open. Later commands use the saved SDK path.
Continue to run build commands from the SDK repository root.

Your effect contains these files:

| File | What you control |
| --- | --- |
| `dsp.c` | Audio processing and reset functions |
| `parameters.json` | Identity, controls, state size, and host compatibility |
| `CMakeLists.txt` | Build settings and developer identity |
| `ui.c` (optional) | Custom editor drawing |

The gain template has no `ui.c`. The host supplies its default parameter editor.
Start with this editor while you learn the DSP interface.

For work inside this SDK repository, follow [AGENTS.md](AGENTS.md).
Use a task worktree and keep its build output inside that worktree.
Your independent effect needs only the public SDK, not host or firmware source.

## 5. Set the identity and controls

Open `../my-first-block/parameters.json` in your editor.
Read the existing fields before you change them.

| Field | Meaning | First-project action |
| --- | --- | --- |
| `id` | Permanent effect identity | Replace `yourcompany.youreffect` before distribution |
| `name` | Name in the host | Choose a display name |
| `package_version` | Version of your package | Keep `1.0.0` for this exercise |
| `minimum_host_version` | First permitted host version, inclusive | Keep the tested lower bound |
| `maximum_host_version` | First excluded host version | Keep the tested upper bound |
| `process`, `reset` | Names of your C functions | Keep the gain callback names initially |
| `state_words` | Number of 32-bit state words per channel | Keep `1` for gain |
| `parameters` | Ordered list of controls | Start with the existing gain control |

Choose a stable namespaced ID that belongs to you.
Use letters, digits, dots, underscores, or hyphens.
Set `DEVELOPER` in `CMakeLists.txt` to your developer identity.
Replace both identity placeholders before you share the package.
Do not use an official `forgeefx.*` identity.
You can retain the placeholders for this unpublished exercise.

Omit `index` for a new effect. The SDK uses -1 by default.
The other catalog indexes are reserved.
Keep the SDK export name `forgeefx_get_block_api` unchanged.

The template permits host versions `0.1.0 <= host < 0.2.0`.
These bounds describe compatibility, not the newest available host version.
Test each intended host release before you widen the bounds.
Package version, host version, and ABI version have different purposes.

### Make your first change

1. Find the `GAIN` parameter in the copied `parameters.json`.
2. Change its `default` from `100` to `50`.
3. Save the file.

```json
"parameters": [
  {"name": "GAIN", "unit": "%", "min": 0, "max": 200, "default": 50}
]
```

This fragment replaces the existing `parameters` field. It is not a complete source manifest.
The new default requests half gain for a new instance with no saved parameter values.
Saved presets can restore a different value.

Each block declares 1 to 64 parameters.
Keep each default between its minimum and maximum.
Keep bounds within -1,000,000 to 1,000,000 and the range width at most 1,000,000.
For choice controls, supply one `labels` entry for each integer in the inclusive range.

Parameter `name` is a persistent key. `display_name` is an optional visible label.
After release, preserve parameter keys, order, ranges, and meaning.
See the [source manifest schema](sdk/parameters.schema.json) for the available fields.

## 6. Understand the gain code

Open the copied `dsp.c`.
The host calls `gain_process` for each input sample:

```c
int gain_process(int sample, int *params, int *state);
void gain_reset(int *state);
```

`params[0]` holds the first declared parameter, `GAIN`.
Parameter positions start at zero and follow the source manifest order.
Treat these values as host-owned inputs.

Audio uses signed Q16 samples at 48,000 samples per second.
In Q16, 65,536 represents 1.0. For example, 32,768 represents 0.5.
The gain template multiplies the sample by the gain percentage, then divides by 100:

```c
int64_t output = (int64_t) sample * params[0] / 100;
```

The cast widens the multiplication before it occurs.
This prevents overflow for the template's permitted inputs.
The return expression limits the result to -65,536 through 65,535.
This limit is **saturation**. It prevents an excessive result from wrapping to an unrelated value.

| Input sample | Gain | Output sample | Meaning |
| --- | --- | --- | --- |
| 32,768 | 0% | 0 | Silence |
| 32,768 | 50% | 16,384 | Half amplitude |
| 32,768 | 100% | 32,768 | Unchanged amplitude |
| 32,768 | 200% | 65,535 | Limited positive output |

Gain needs no audio history. Its reset function clears the one reserved state word.
A delay needs state to remember earlier samples.
The host supplies separate state arrays for channels and instances.

When you develop your own DSP:

1. Put all mutable instance data in the host-provided state arrays.
2. Reserve enough words with `state_words`.
3. Make reset restore the same starting state after any previous processing.
4. Use widened arithmetic and define overflow behavior.
5. Keep channels and instances independent.
6. Match the callback signatures in [forgeefx_block.h](sdk/include/forgeefx_block.h).

Do not allocate memory, access files, lock, or wait inside process or reset callbacks.
Do not load models, log, or draw inside these callbacks.
These operations can interrupt audio processing.
Do not pass C++ objects or exceptions across the C interface.
Do not change ABI struct packing or memory ownership.

## 7. Build your copied block

Run the commands for your operating system from the SDK repository root.
`FORGEEFX_SDK_DIR` must identify the `sdk` subfolder, not the repository root.

On Windows PowerShell:

```powershell
cmake -S ../my-first-block -B build/first-block -DCMAKE_BUILD_TYPE=Release "-DFORGEEFX_SDK_DIR=$sdkRoot/sdk"
cmake --build build/first-block --config Release --parallel
```

On macOS:

```sh
cmake -S ../my-first-block -B build/first-block -DCMAKE_BUILD_TYPE=Release "-DFORGEEFX_SDK_DIR=$sdk_root/sdk"
cmake --build build/first-block --config Release --parallel
```

**Expected result:** Your package appears in `build/first-block/dist/<your-effect-id>.fxblock/`.
The folder name follows the source `id`, not the project folder name.
If you retained the placeholder ID, the folder is `yourcompany.youreffect.fxblock`.

Run configuration and compilation again after source metadata changes.
Keep generated files in the build folder.
The CMake helper includes `dsp.c`, the generated descriptor, and optional `ui.c`.
See the Development Guide before you add other implementation files.

## 8. Validate the actual binary

The root build created the validator in `build/native/sdk`.
The validator loads the compiled binary and checks the SDK contract.
It checks metadata, reset, independent state, and parameter extremes.
It does not establish audio quality or correct editor appearance.

For Windows with Visual Studio:

```powershell
& ./build/native/sdk/Release/forgeefx_block_validator.exe ./build/first-block/dist/yourcompany.youreffect.fxblock/bin/windows-x64/block.dll
```

For Windows with Ninja:

```powershell
& ./build/native/sdk/forgeefx_block_validator.exe ./build/first-block/dist/yourcompany.youreffect.fxblock/bin/windows-x64/block.dll
```

For macOS with a single-configuration generator and an Apple Silicon build:

```sh
./build/native/sdk/forgeefx_block_validator ./build/first-block/dist/yourcompany.youreffect.fxblock/bin/macos-arm64/block.dylib
```

Replace the package ID if you changed it.
For other Mac builds, use `macos-x64` or `macos-universal` as applicable.
**Expected result:** The validator reports success and exits with code 0.

Inspect the generated `manifest.json` without changing it.
Confirm the effect ID, developer ID, package version, host bounds, and platform binary path.
Confirm that the compiled package describes a gain default of 50.

Add focused DSP tests when you change the algorithm.
Use [GainTests.c](tests/GainTests.c) as a starting point for assertions and expected sample values.
The root gain test checks the bundled template, not your separate project.
Give your project its own tests for new behavior.

## 9. Load and test in DL-1

Use a DL-1 host version inside your package's declared range.
Use a binary compatible with the host's operating system and architecture.
The SDK does not build or install the host application.

1. Use your host installation's configuration to identify a Blocks directory that the host scans.
2. Close DL-1 and any DAW that has loaded the block.
3. Copy the complete `.fxblock` folder into that directory.
4. Restart the host.
5. Add a new instance of your block.
6. Check that the gain starts at 50% when no preset overrides it.
7. Send a quiet test signal through the block.
8. Compare 0%, 50%, 100%, and 200% gain.

Do not replace a binary while the host has it loaded.
Install one package for each effect ID to avoid duplicate identities.

Check silence, repeated reset, rapid control changes, and multiple instances.
Check mono and stereo behavior as applicable.
Check presets, automation, bypass, and any audio tails in each supported host and DAW.
Record automated results separately from listening and host checks in your project's `VERIFICATION.md`.
Record unrun checks as unrun.

## 10. Add an editor when the DSP works

The default editor is sufficient for the first exercise.
For a custom editor, read [Development Guide: Add an optional editor](Development%20Guide.md#7-add-an-optional-editor).
The source `ui.c` function takes `const BlockUI *` and matches the manifest's `render` symbol.

Pair `block_ui_begin(ui)` with `block_ui_end(ui)`.
Draw only between these calls.
Do not retain UI or drawing-service pointers after rendering.
Use the logical 160 by 80 canvas and the SDK layout helpers.

Inspect screenshots from the actual host after artwork changes.
The validator's drawing substitutes cannot verify the appearance.

Use these examples for the next step:

| Example | What it teaches |
| --- | --- |
| [Gain](examples/gain) | One parameter and simple sample arithmetic |
| [Delay](examples/delay) | A circular buffer, feedback, and a custom editor |
| [Phaser](examples/phaser/README.md) | Modulation and filter state |
| [Responsive UI](examples/responsive_ui/README.md) | Waveform drawing and host-managed controls |

## 11. Prepare a package for distribution

1. Replace all template identity values.
2. Set the package version for your release.
3. Build separately for Windows and macOS as required.
4. Validate each actual platform binary.
5. Test each supported host and architecture.
6. Complete the required Mac signing and notarization for your distribution workflow.
7. Ship the complete `.fxblock` folder.

Windows builds do not produce Mac binaries.
When you combine platform binaries, use identical source, identity, package version, and metadata.
Keep builds, credentials, local tool paths, and dependencies out of source commits.

## Troubleshooting

| Problem | Check |
| --- | --- |
| A command is unavailable | Install the tool and reopen the correct terminal |
| CMake cannot find a compiler | Use the x64 developer terminal on Windows, or install Xcode command-line tools on Mac |
| CMake cannot find Python | Check Python 3 availability and use `-DPython3_EXECUTABLE="/absolute/path/to/python"` if needed |
| CMake cannot find the SDK | Point `FORGEEFX_SDK_DIR` to the folder containing `cmake/ForgeEFXBlock.cmake` |
| A generator or architecture conflicts | Configure in a new build folder |
| Metadata generation fails | Check JSON syntax, parameter ranges, callback names, and host bounds |
| The validator path does not exist | Check the generator's `Release` subfolder and build the repository root first |
| The host cannot find the block | Check its scan directory, package nesting, duplicate IDs, architecture, and host bounds |
| Gain still starts at 100% | Rebuild, install the correct package, and create an instance without saved parameter values |
| Audio clicks during control changes | Check parameter smoothing in your DSP; the gain template does not smooth changes |

For the full interface and build options, continue with the [Development Guide](Development%20Guide.md) and [SDK reference](sdk/README.md).
