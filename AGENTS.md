# AGENTS

## Development Defaults

- Use CMake helper scripts in `scripts/` for configure/build/format actions.
- Prefer `rg` for fast file discovery/search.
- Keep new DSP code in `src/dsp/`, UI in `src/ui/`, and parameters in `src/util/`.
- Add tests for behavior changes in `tests/` and run `ctest --test-dir build --output-on-failure`.

## Formatting and Style

- C++ standard: C++17
- Formatting: `.clang-format`
- Editor conventions: `.editorconfig`

## Build Targets

- Plugin target: `QuadraBass`
- Test target: `quadrabass_tests`
