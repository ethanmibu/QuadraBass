# Changelog

All notable feature and behavior changes are recorded here.

## 2026-02-25

- Removed crossover controls and crossover processing from plugin UI/DSP/tests.
- Standardized full-band quadrature width behavior for mono input widening.
- Consolidated project documentation policy to `README.md` and `CHANGELOG.md`.
- Added FIR/IIR mode-switch rollout plan and acceptance targets in `README.md`.
- Implemented user-facing `Hilbert Mode` (`IIR` default, optional `FIR`).
- Added FIR latency reporting and cross-sample-rate FIR acceptance tests.
- Fixed AU/VST3 bundle packaging to always include `Contents/Info.plist`
  so hosts can reliably discover rebuilt plugins.
