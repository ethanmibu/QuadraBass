# QuadraBass

Hilbert-transform DSP plugin built with JUCE and CMake.

## Status

Current active goal: take mono program material and widen continuously from
mono (`0%`) to near-full quadrature (`100%`) while keeping behavior consistent
across the spectrum within defined tolerances.

Current Hilbert implementation modes: `IIR` (default, low-latency all-pass
cascade) and `FIR` (high-accuracy quadrature with reported latency).

Width behavior by mode:

- `FIR`: uses a linear decorrelation width law and balanced stereo matrix
  (including harmonic-rich content handling).
- `IIR`: keeps legacy low-latency behavior and is not held to the same
  decorrelation/channel-balance accuracy targets as FIR.

Project documentation policy:

- Feature behavior and usage notes belong in this `README.md`.
- Change history belongs in `CHANGELOG.md`.

## FIR/IIR Mode Plan (Implemented)

This plan is only valid if mode switching does not materially change plugin
functionality (width behavior, mono collapse, and compatibility checks).

1. Lock current behavior as baseline:
   - Capture IIR reference metrics from current compliance tests.
   - Add explicit regression checks so the default path stays bit-stable enough.
2. Add mode abstraction in DSP:
   - Extend `HilbertQuadratureProcessor` with `Mode::IIR` and `Mode::FIR`.
   - Keep default mode as `IIR` for backward compatibility.
3. Implement FIR Hilbert branch:
   - Add FIR odd-symmetric Hilbert coefficients and convolution state.
   - Match branch gain as closely as practical to current output law.
4. Handle latency safely:
   - Report FIR latency via plugin latency APIs.
   - Align I/Q paths so stereo matrix input contract is unchanged.
5. Add non-breaking parameter/state support:
   - Add a mode parameter with safe default to `IIR`.
   - Ensure old presets/sessions load identically.
6. Verify no functional regressions:
   - Run full existing suite on both modes.
   - Add cross-mode checks for width consistency and fold-down tolerance.
7. Gate release:
   - If FIR fails compatibility thresholds, keep FIR disabled/internal.
   - Only expose switching in UI after all gates pass.

Current implementation status:

- `Hilbert Mode` is user-facing in the plugin UI with `IIR` and `FIR` options.
- Default mode remains `IIR` for backward compatibility with existing sessions.
- FIR mode reports plugin latency and aligns I/Q paths for consistent stereo
  matrix behavior.
- Automated acceptance checks run for `44.1/48/96 kHz` and are part of the
  test suite (`tests/HilbertQuadratureTests.cpp`).

### Acceptance Targets For FIR Mode

- Certified sample rates: `44.1 kHz`, `48 kHz`, `96 kHz`.
- I/Q phase accuracy (main band): `|phase-90deg| <= 3deg` (95th percentile),
  max `<= 8deg` over `30 Hz .. 0.45*Fs`.
- I/Q phase accuracy (edge band): max `<= 12deg` over `0.45*Fs .. 0.48*Fs`.
- I/Q magnitude match (main band): `<= 0.5 dB` (95th percentile), max `<= 1.5 dB`.
- Width `0%`: stereo difference `<= -60 dB` relative to input RMS.
- Width `100%`: per-band correlation (1/3-oct style probes, `40 Hz .. 12 kHz`)
  remains in `[-0.15, +0.15]` with cross-band std-dev `<= 0.08`.
- Mono collapse `(L+R)/2`: `<= +/-1.0 dB` error in `40 Hz .. 12 kHz`,
  relaxed to `<= +/-2.0 dB` above that.
- Mode-switch compatibility: `IIR` remains default and backward-compatible with
  existing sessions.

## Target Formats

- macOS: AU, VST3
- Windows: VST3

## Prerequisites

- CMake 3.21+
- C++17 toolchain
- JUCE submodule in `third_party/JUCE`

### macOS

- Xcode + command line tools
- Ninja (recommended)
- clang-format

### Windows

- Visual Studio 2022 (Desktop development with C++)

## Clone + JUCE Setup

```bash
git clone https://github.com/ethanmibu/QuadraBass.git
cd QuadraBass
git submodule update --init --recursive
```

## Build

### macOS

```bash
./scripts/setup_macos.sh
./scripts/configure.sh -G Ninja
./scripts/build.sh
```

### Windows (PowerShell)

```powershell
.\scripts\setup_windows.ps1
bash ./scripts/configure.sh -G "Visual Studio 17 2022" -A x64
bash ./scripts/build.sh --config Release
```

## Test

```bash
./scripts/configure.sh -DBUILD_TESTING=ON
./scripts/build.sh --target quadrabass_tests
ctest --test-dir build --output-on-failure
```

## Plugin Discovery (macOS)

- AU and VST3 bundle metadata (`Contents/Info.plist`) is copied during plugin
  post-build packaging.
- After replacing installed bundles in
  `~/Library/Audio/Plug-Ins/{Components,VST3}`, fully restart your DAW and run
  a plugin rescan.
- If AU is still missing, restart the registrar and validate:

```bash
killall -9 AudioComponentRegistrar
auval -v aufx QdBs EtMb
```

## Formatting

```bash
./scripts/format.sh apply
./scripts/format.sh check
```

## License

Licensed under GNU Affero General Public License v3.0. See `LICENSE`.
