# ForgeEFX block SDK 1.0.0

## Build your first effect

Copy `examples/gain` from the SDK release into your own project. Give it a stable
namespaced ID such as `yourcompany.youreffect`, and set the `DEVELOPER` CMake
argument to your developer ID. Unnamespaced official catalog IDs are reserved.
Keep the SDK directory anywhere accessible to the build:

```sh
cmake -S your-effect -B build -DFORGEEFX_SDK_DIR=/path/to/sdk
cmake --build build --config Release
```

Install the complete `build/dist/yourcompany.youreffect.fxblock` folder in the
full ForgeEFX host. Build separately for Windows x64 and Mac. Trial approval is controlled by the host's compiled JSON ID allowlist. Blocks
cannot grant themselves trial access with a flag; compatible updates with an
approved ID can load. This example is not approved by the default trial.

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
host compatibility and trial ID approval are independent. ID approval is product
policy rather than cryptographic publisher authentication.

## Binary contract

`include/forgeefx_block.h` is authoritative. One C export,
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
serialization, model/file services and live module unloading are outside ABI v1.

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
