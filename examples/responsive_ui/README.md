# Responsive UI demo

A native custom editor built from the gain template. The gain DSP is unchanged:
0..200 percent, unity by default, with Q16 saturation. Each channel uses its own
host-provided state. There are no additional dependencies or SDK changes.

The editor uses the SDK's monochrome artwork and control conventions:

- A gain knob follows the parameter, fills during edits, and has a focus underline.
- The OUTPUT trace uses the host's captured waveform and shared display range.
  Missing offline captures show a flat line; there is no synthetic signal.
- LVL shows the host's compressed output activity, not a calibrated dB meter.
- Bypass and loading suppress the trace. The host draws its status overlay and
  makes the activity meter idle.

Responsive layout belongs to the host: `block_ui_begin` prepares a logical
160x80 canvas, and `block_ui_end` fits artwork into the display's content area
and draws responsive parameter controls. This demo keeps its drawing between
`BLOCK_UI_TOP` and `BLOCK_UI_BOTTOM`; it does not assume physical LCD dimensions.
With one parameter it uses one artwork page. It demonstrates native host
scaling and live feedback, rather than browser breakpoints or window resizing.

## Build

The repository's root build includes this demo. To build it independently:

```sh
cmake -S examples/responsive_ui -B build/responsive-ui -DCMAKE_BUILD_TYPE=Release
cmake --build build/responsive-ui --config Release --parallel
```

When copying the example outside this checkout, pass
`-DFORGEEFX_SDK_DIR=/absolute/path/to/sdk` during configuration.

The output is `build/responsive-ui/dist/yourcompany.responsive_ui_demo.fxblock/`.
Both `yourcompany` and `yourcompany.responsive_ui_demo` are unpublished example
placeholders. Replace the CMake developer and source manifest ID before
distribution. The distinct demo ID lets it coexist with the original gain example.
Its declared host compatibility is `0.1.0 <= host < 0.2.0`; trial approval remains
controlled by the host.

## Try in DL-1

Close DL-1 and any DAW hosting it before installing the entire package in a
scanned Blocks directory. Restart the full compatible host, select SDK UI DEMO,
and feed audio through it. Adjust GAIN through 0%, 100%, and 200%; observe the
knob, edit readout, trace, and level. Toggle bypass and test silence, mono/stereo,
multiple instances, presets, automation, and each supported display layout.

Actual host screenshots are required to verify artwork and host scaling.
Automated UI tests check calls and bounds with service substitutes; they do not
render the host's fonts, overlays, chrome, or display fit. See the root
`VERIFICATION.md` for completed checks and remaining host checks.
