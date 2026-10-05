# ForgeEFX DL-1 block development

Read `Development Guide.md`, `sdk/README.md`, and the source manifest schema
before making changes. This repository contains the public SDK and gain template.
Host, firmware and effect-catalog repositories are read-only references, never
build dependencies. Do not copy models, dependencies or proprietary assets here.

## Scope and identity

- For a new effect, copy `examples/gain` into an effect folder and edit that
  folder's `dsp.c`, `parameters.json`, `CMakeLists.txt`, and optional `ui.c`.
  Keep SDK internals unchanged unless the task explicitly concerns the SDK.
- Replace `yourcompany.youreffect` in `parameters.json` with the developer's
  stable namespaced effect ID, and replace `DEVELOPER yourcompany` in CMake.
  Ask for the intended identity if it is not supplied; retain placeholders for
  an unpublished prototype. Do not invent an official ForgeEFX identity.
- Never edit generated `descriptor.c`, package `manifest.json`, or build output.
  Change source metadata and rebuild. Leave the SDK ABI export name unchanged.
- Preserve released IDs, parameter `name` keys, ordering, ranges and meaning.
  New effects omit `index` (default -1); legacy catalog indexes are reserved.
- Host version bounds declare compatibility, not trial eligibility. The host's
  compiled ID allowlist owns trial approval; never add a trial-access flag.

## DSP and editor rules

- Use C11/C++20, four spaces, and surrounding C conventions. Samples are signed
  Q16 at 48 kHz. Use widened intermediates and defined overflow behavior.
- No allocation, file I/O, locks, blocking work, model loading, logging or drawing
  inside process/reset callbacks. All mutable instance state belongs in the
  host-provided state arrays, with deterministic reset and independent channels.
- Match callback signatures in `sdk/include/forgeefx_block.h`. No exceptions,
  STL objects, struct packing changes or cross-module allocation ownership.
- Parameters number 1..64. Bounds stay within +/-1,000,000 and span at most
  1,000,000. Defaults must be in range; choices must match the inclusive range.
- Custom `ui.c` implements the manifest render symbol with `const BlockUI *`.
  Pair `block_ui_begin(ui)` and `block_ui_end(ui)` and draw only between them.
  UI/service pointers expire after rendering. Inspect actual host screenshots
  when changing artwork; the validator's drawing stubs do not verify appearance.

## Workflow and validation

- Use a task branch/worktree under `.worktrees/<task>-<role>/`, created from
  `main`. State file ownership and use a build directory inside that worktree.
  Commit explicit source files. Merge accepted work in an integration worktree,
  validate, then fast-forward `main` and leave the primary checkout there.
  Remove only clean, merged worktrees. Do not push unless requested.
- Run the root CMake build and CTest commands in `Development Guide.md`.
  For a new block, also build its standalone CMake project and run
  `forgeefx_block_validator` against its actual compiled binary.
- Inspect the generated package identity, host version bounds and platform
  binary. Check reset, silence, parameter extremes, multiple instances, mono
  and stereo as applicable. Add focused DSP tests for new behavior.
- Record commands and automated results in `VERIFICATION.md`, separately from
  listening, Windows host, macOS host and DAW checks. Never claim an unrun check.
- Do not commit builds, archives, credentials, local tool paths or dependencies.
  Keep distribution packages free of placeholders and ship the entire
  `.fxblock` folder. Never replace a library while the host has it loaded.
