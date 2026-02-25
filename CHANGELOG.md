# Changelog

All notable feature and behavior changes are recorded here.

## 2026-02-25

- Removed crossover controls and crossover processing from plugin UI/DSP/tests.
- Standardized full-band quadrature width behavior for mono input widening.
- Consolidated project documentation policy to `README.md` and `CHANGELOG.md`.
- Added FIR acceptance targets in `README.md`.
- Implemented user-facing `Hilbert Mode` (`FIR` default, optional `IIR`).
- Added FIR latency reporting and cross-sample-rate FIR acceptance tests.
- Fixed AU/VST3 bundle packaging to always include `Contents/Info.plist`
  so hosts can reliably discover rebuilt plugins.
- Updated width matrix so deterministic phase-law and channel-balance behavior
  applies to `FIR` mode; `IIR` keeps legacy low-latency behavior.
- Added harmonic-rich (`square`/`saw`) compliance coverage for FIR width behavior.
- Removed the `FIR/IIR Mode Plan (Implemented)` section from `README.md`.
- Tightened `.gitignore` for agent/planning artifacts and local Codex metadata.
