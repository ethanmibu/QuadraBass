# QuadraBass

Hilbert stereo widener plugin for bass sources, built with JUCE and CMake.

## Status

Repository initialization scaffold. DSP and UI are placeholders pending implementation milestones.

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

## Formatting

```bash
./scripts/format.sh apply
./scripts/format.sh check
```

## License

Licensed under GNU Affero General Public License v3.0. See `LICENSE`.
