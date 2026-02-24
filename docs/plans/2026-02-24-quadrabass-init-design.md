# QuadraBass Repository Initialization Design

**Date:** 2026-02-24  
**Project:** QuadraBass (Hilbert Stereo Widener)  
**Scope:** Repository initialization only (no production DSP implementation yet)

## 1. Concept and Goal

QuadraBass is a JUCE-based audio plugin project focused on mono-compatible stereo widening for bass content using quadrature phase techniques (Hilbert-style I/Q architecture). This design defines how to initialize the repository, build system, CI, and project scaffolding so DSP and UI development can proceed cleanly.

## 2. Selected Approach

### Options considered

1. EQInfinity-style baseline (selected)
2. Clean-room modern JUCE template
3. Hybrid baseline

### Why option 1

- Fastest and lowest risk path to a reliable cross-platform plugin setup.
- Reuses proven CMake, script, and CI conventions already validated in a sibling project.
- Keeps team workflow consistent while allowing later targeted refactors.

## 3. Architecture (Initialization Phase)

- Build a JUCE + CMake plugin project named `QuadraBass`.
- Add JUCE as a git submodule at `third_party/JUCE`.
- Configure platform plugin formats:
  - macOS: `AU`, `VST3`
  - Windows: `VST3`
- Apply `AGPL-3.0` licensing.
- Create and link GitHub repository: `ethanmibu/QuadraBass` (public).

## 4. Planned Repository Structure

- `CMakeLists.txt`
  - JUCE plugin target and project options.
  - Configurable plugin formats and copy-after-build behavior.
  - Optional test build target.
- `src/`
  - `PluginProcessor.*`, `PluginEditor.*`
  - `dsp/` placeholders for band-split, Hilbert, and stereo matrix stages.
  - `ui/` placeholder `StereometerComponent`.
  - `util/` parameter IDs and layout declarations.
- `tests/`
  - Baseline unit test executable validating toolchain and future DSP insertion points.
- `scripts/`
  - `configure.sh`, `build.sh`, `format.sh`, plus platform setup scripts.
- `.github/workflows/`
  - macOS and Windows CI for format/build/test.
- Root hygiene/config:
  - `.gitignore`, `.editorconfig`, `.clang-format`, `README.md`, `LICENSE`, `AGENTS.md`.

## 5. DSP and UI Boundaries for Init

### DSP boundaries

- Define APVTS parameter schema and ranges:
  - Crossover: 20 Hz to 500 Hz
  - Width: 0% to 100%
  - Phase Angle: 0 deg to 180 deg (default 90 deg)
  - Phase Rotation: -180 deg to +180 deg
  - Gain: -inf dB to +12 dB
- Add class interfaces and placeholder wiring only:
  - `HilbertProcessor`
  - `BandSplitProcessor`
  - `StereoMatrixProcessor`
- Do not implement final IIR/FIR Hilbert algorithms in this pass.

### UI boundaries

- Add baseline editor layout for controls and visualizer region.
- Add placeholder stereometer component API and paint stub.
- Defer full XY goniometer rendering behavior to later implementation milestones.

## 6. Error Handling and Guardrails

- Clamp all parameter values to supported ranges.
- Ensure pass-through/bypass-safe behavior if DSP stages are not fully initialized.
- Fail fast with clear CMake error if JUCE submodule is missing.

## 7. Verification Criteria for Initialization Completion

Initialization is complete when all are true:

1. Local configure/build succeeds.
2. Test target compiles and runs.
3. CI workflows exist and execute format/build/test pipelines.
4. `origin` points to `ethanmibu/QuadraBass`.
5. Initial scaffold commit is pushed to GitHub.

## 8. Out of Scope (This Pass)

- Production-quality Hilbert all-pass design.
- FIR Hilbert option and latency mode switching.
- Full stereometer vector/circle fidelity visualization logic.
- Final UI styling/polish and preset system.
