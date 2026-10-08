# ForgeEFX block SDK 1.0.0

Visit [www.forgeefx.com](https://www.forgeefx.com/) for ForgeEFX product information.

## Build your first effect

For a step-by-step introduction, read [Getting Started](../Getting%20Started.md).

Copy `examples/gain` from the SDK release into your own project. Give it a stable
namespaced ID such as `yourcompany.youreffect`, and set the `DEVELOPER` CMake
argument to your developer ID. Unnamespaced official catalog IDs are reserved.
Keep the SDK directory anywhere accessible to the build:

```sh
cmake -S your-effect -B build -DFORGEEFX_SDK_DIR=/path/to/sdk
cmake --build build --config Release
```

Install the complete `build/dist/yourcompany.youreffect.fxblock` folder in the
ForgeEFX host. Build separately for Windows x64 and Mac.

An effect folder contains `dsp.c`, `parameters.json`, and optionally `ui.c`.
`parameters.schema.json` describes the source manifest; the generator also checks
cross-field constraints such as parameter defaults and choice counts.
Additional implementation can be included through local headers; shared include
directories are passed as `INCLUDE_DIRECTORIES` to `forgeefx_add_block`. A custom
editor includes `block_ui.h`, implements its manifest's render symbol and draws
between `block_ui_begin` and `block_ui_end`. Omitting `ui.c` uses the default editor.

To build the SDK's validator separately:

```sh
cmake -S sdk -B validator-build
cmake --build validator-build --config Release
```

Run it against the package's platform binary. Archive creation, compilation and
validation do not publish or upload anything.

## Host compatibility

Optional `minimum_host_version` (inclusive) and `maximum_host_version` (exclusive)
use strict `major.minor.patch` versions. The generator defaults to `0.1.0` and
`0.2.0`, and rejects malformed or empty ranges. The host checks this range before
loading the library and also requires a compatible SDK ABI. Package versions,
host compatibility and SDK ABI versions are independent.

## Analog circuit blocks

The SDK includes `include/expansion_analog.h`, the same independent DSP helper
header shipped by the Blocks SDK. It requires only standard C library headers;
no catalog or host checkout is needed. Include it with `#include "expansion_analog.h"`.

Model circuit signals in physical volts. The measured DL-1/MOTU convention is
**5.62 V peak per digital full scale at the input jack** (`AX_IN_VOLTS`) and
**4.04 V peak per digital full scale at the output jack** (`AX_OUT_VOLTS`), with
MOTU guitar input gain at +0 dB. These values come from the BOSS BD-2 measurements
of 6 October 2026. They are peak sine amplitudes, not RMS; mixing the two
conventions introduces a 3.01 dB error. See [source provenance](../THIRD_PARTY_NOTICES.md).

Convert once at each circuit boundary, inside your process callback:

```c
float input_volts = axInput(sample) * AX_IN_VOLTS;
float output_volts = circuit_step(input_volts, params, state);
return axOutput(output_volts / AX_OUT_VOLTS);
```

`circuit_step` represents your own circuit implementation, not an SDK function.
`axInput` converts Q16 to normalized digital amplitude, clamped to
[-1, 65535/65536]. `axOutput` clamps normalized output to the same range and
converts to Q16 by truncating toward zero; non-finite output becomes silence.
Neither helper applies the voltage constants automatically.

Keep component gains, resistor dividers, supply rails, diode thresholds and
detectors in physical units. Blend clean and distorted circuit branches in volts
before output conversion. Restore volts after any internal normalization.
Preserve real circuit recovery amplifiers and clipping, without adding loudness
normalization, makeup trims or synthetic soft converter ceilings. Document quiet
noon LEVEL settings rather than adjusting them to digital unity.

Physical unity means equal input and output jack volts. With these converters,
that gives about **+2.87 dB in digital samples** before clipping. Software bypass
stays at digital unity. Digital effects, including the bundled gain, delay,
phaser and responsive UI examples, keep their normalized Q16 processing and do
not apply these voltage conversions. Identify analog-style algorithms without a
defensible voltage mapping as uncalibrated; do not invent volts or claim a
hardware match.

When migrating an existing circuit block, preserve its ID, parameter keys,
ordering, ranges, defaults and legacy index. Saved patches retain settings but
may sound more driven and change level. Compare before/after peak jack levels
at the same input gain and knob settings, as well as CPU cost. Update circuit
plots and live-peak conversions when their voltage assumptions change; UI peak
and waveform fields still contain Q16 digital samples. Inspect affected editors
in the actual host. Keep automated results, listening, Windows host, Mac host
and DAW checks separate in `VERIFICATION.md`.

Calibration is a DSP convention, not an ABI or manifest change. Keep provenance
in documentation and use host-version bounds only for loading compatibility.

## Binary contract

`include/forgeefx_block.h` is authoritative. One required C export,
`forgeefx_get_block_api(requested_abi)`, returns the immutable module descriptor
or null for unsupported versions. ABI v1 uses naturally aligned platform structs,
32-bit `int`, and 64-bit Windows/Mac targets. Never change calling conventions or
packing. No STL, JUCE objects, exceptions or ownership of allocated memory cross
the interface. Descriptor strings/tables remain valid until module unload.

The host supplies 64 integer parameters and independently owned, initially zeroed
state per channel. `process(sample, params, state)` receives signed Q16 samples at
48000 Hz. `reset(state)` must restore deterministic behavior. The optional stereo
callback receives left/right samples, params, separate channel states and output
pointers. `state_words` counts 32-bit words per channel; `state_alignment` defaults
to 8 and can be 4 or 8 in v1. Preserve existing parameter ordering and meaning.
Parameter bounds must remain within -1,000,000..1,000,000, with a range width no
greater than 1,000,000. This keeps v1 integer UI normalization, formatting and
editing arithmetic within range. Encode wider DSP quantities through a scaled
parameter rather than exposing the full 32-bit integer range directly.

DSP/reset callbacks must not allocate, block, perform file I/O, load models or use
drawing services. Do not store per-instance state in module globals. The host owns
routing, sample-rate conversion and saved parameter values. General custom state
serialization, model/file loading and live module unloading are outside ABI v1.

`render(ui, services)` runs on the message thread. Both arguments expire on return.
The SDK wrapper installs the host service table in module-local thread-local
storage and restores the previous table after rendering. Never retain UI/service
pointers or call drawing helpers outside render. The host owns physical display
dimensions and scaling; author artwork on the logical 160x80 canvas.

The drawing service field order is fixed for ABI v1. ABI compatibility, package
versions and persistent parameter identities are separate. Any incompatible
binary layout change requires a new ABI. The host retains modules while any
processing/rendering callback could still reference them.

Native blocks execute with host privileges; a malformed block can crash the DAW.
The validator catches common integration errors but does not establish trust or
audio quality. Sign Mac releases and test supported DAWs before distribution.

## Host content folders

Include `forgeefx_host_services.h` and call
`forgeefx_host_get_folder(FORGEEFX_FOLDER_NAM, buffer, capacity, &required)` or
use `FORGEEFX_FOLDER_IR`. The host returns its current absolute UTF-8 path. The
caller owns the buffer; `required` is mandatory and includes the terminating NUL.
Pass a null buffer and capacity zero to query the size. Size queries and small
buffers return `FORGEEFX_HOST_BUFFER_TOO_SMALL`; retry if preferences change
between sizing and copying. Success is `FORGEEFX_HOST_OK`. Other results are
`FORGEEFX_HOST_UNAVAILABLE`, `FORGEEFX_HOST_INVALID_ARGUMENT` and
`FORGEEFX_HOST_ERROR`. Failed calls empty a supplied nonempty buffer and clear
the required size, except that small-buffer results retain the needed size.

The CMake helper compiles `src/host_bridge.c` into every module, with or without
an editor. A supporting host binds the optional `forgeefx_set_host_services`
export after validating the descriptor, before block callbacks. The immutable
table and its context remain valid through all module calls; binding must not
race queries. The bridge rejects unsupported service versions, short structs
and missing callbacks. Older hosts leave the service unavailable. Older modules
can omit the export. Existing block/render ABI v1 layouts remain unchanged.

Queries work without an open editor. Query again to observe folder changes.
Use only UI or worker threads: never query or perform file/model work inside
process/reset. The service provides paths, not content loading or serialization.
No heap ownership crosses the ABI. Drawing-service pointers retain their separate,
render-only lifetime; do not use them for content access.

Handle unavailable services at runtime, including older builds sharing a host
version. If folders are required, set the package's `minimum_host_version` to the
first supporting release once assigned; a version range does not replace the
capability check. The service ABI version is independent of the host version.
See the [development guide](../Development%20Guide.md#read-the-hosts-nam-and-ir-folders)
for a complete C allocation/retry example and content-loading constraints.
