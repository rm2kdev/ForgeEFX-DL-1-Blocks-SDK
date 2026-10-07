# SDK source provenance

This SDK is exported from the `sdk/` and `examples/gain/` directories of
[ForgeEFX DL-1 Blocks](https://github.com/rm2kdev/ForgeEFX-Dl-1-Blocks),
source revision `303acc6e11bc018e3600cf28e20dc22fad0af3c8`.
The compatibility and Mac target-selection tests come from the same revision.
The public template replaces example publisher identities with placeholders;
the package generator also fixes unused render-symbol validation for effect
folders containing hyphens. The public guides and root build/tests are added here.

The ABI headers, native Q16 helper, package generator, CMake helper, rendering
bridge, validator and gain example are the project's SDK implementation.
The display and block-UI declarations retain the conventions of the original
FreeFX firmware-derived editor interface. Their existing names and header guards
are preserved for source compatibility.

This export includes no firmware implementation, effect catalog, JUCE, NAM Core,
Eigen, JSON library, fonts, captured models, impulse responses or host assets.
Python tooling uses the standard library. CMake, Python, the compiler and platform
SDK are external build tools and are not vendored here.

The standalone `sdk/include/expansion_analog.h` is copied unchanged from
`blocks/expansion_analog.h` at Blocks revision
`273879ec9166cb751e229511f1e40879ae86af4d` (6 October 2026), as included by that
repository's SDK archive packager. It contains independent DSP primitives, not
effect models. Its voltage convention and the accompanying authoring guidance
come from that revision's `sdk/README.md` and `CONVERTER_VOLTAGE_AUDIT.md`.
The audit cites BOSS BD-2 measurements from 6 October 2026 with MOTU guitar
input gain at +0 dB: 5.62 V peak input and 4.04 V peak output per digital full
scale. Measurement recordings, circuit models and host tools are not included
or required. This SDK update does not claim fresh hardware measurements.

The full Blocks repository's provenance document covers additional code and
assets outside this export. Those components are not dependencies of this SDK.
