# Minimal phaser demo

Four swept first-order all-pass stages, mixed equally with the dry signal.
No feedback, allocation, external assets or dependencies. The custom editor
uses the SDK's two compact knob controls and the host's parameter footer.

- **RATE:** 1..50 in tenths of Hz (5 = 0.5 Hz; range 0.1..5 Hz).
- **DEPTH:** 0..100%, default 75%. Zero stops the sweep at its centre;
  use the host's bypass to hear the dry signal.

Processing uses signed Q16 at 48 kHz, widened integer arithmetic and full-scale
output saturation. Five state words belong to each channel; the host runs the
mono callback independently for stereo. The triangle LFO resets identically
on both channels. Rapid automation is bounded but is not parameter-smoothed.

Build from the repository root, or standalone:

```sh
cmake -S examples/phaser -B build/phaser -DCMAKE_BUILD_TYPE=Release
cmake --build build/phaser --config Release --parallel
```

For a copied example, supply `-DFORGEEFX_SDK_DIR=/absolute/path/to/sdk`.
The complete package is `build/phaser/dist/yourcompany.phaser.fxblock`.
This is an unpublished prototype identity. Replace `yourcompany.phaser` in
`parameters.json` and `DEVELOPER yourcompany` in CMake before distribution.
Compatibility is `0.1.0 <= host < 0.2.0`; this does not grant trial approval.
See the root `VERIFICATION.md` for checks performed and remaining host checks.
