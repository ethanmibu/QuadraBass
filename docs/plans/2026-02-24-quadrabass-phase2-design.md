# QuadraBass Phase 2 DSP Design

**Date:** 2026-02-24  
**Project:** QuadraBass  
**Scope:** Production DSP + UI behavior for quadrature widening (after Phase 1 scaffold)

## 1. Objectives

- Build a low-latency quadrature stereo widener for bass-focused sources.
- Preserve mono translation by designing against average mono fold-down `(L + R) / 2`.
- Keep low frequencies centered by default through an adjustable Linkwitz-Riley split.
- Provide user feedback with a goniometer and correlation metering.

## 2. Locked Product Decisions

- Two-phase roadmap remains in effect:
  - Phase 1: repository/build/test scaffolding.
  - Phase 2: DSP/UI implementation.
- Hilbert mode for v1: IIR all-pass only (no FIR mode).
- Crossover:
  - Linkwitz-Riley (LR4), `20..500 Hz`.
  - Default enabled at `90 Hz`.
  - Bypass mode for full-band processing.
- Main controls:
  - `crossoverEnabled`, `crossoverHz`, `widthPct`, `outputGainDb`.
- Advanced controls:
  - `phaseAngleDeg` (`0..180`, default `90`).
  - `phaseRotationDeg` (`-180..180`, default `0`).
- Default width: `0%`.
- Plugin targets in this phase: `AU` and `VST3` only.

## 3. Quantified Compatibility Statement

Use this wording in docs/UI copy:

> QuadraBass is designed for mono-compatible widening. In validation tests, fold-down `(L+R)/2` stays within defined level and phase-error tolerances without comb-style cancellations across the target band.

Do not use absolute "100% mono compatible" phrasing.

## 4. Signal Architecture

## 4.1 Input policy

- If input is mono: use channel 0 as source `x[n]`.
- If input is stereo: derive processing source as `x[n] = 0.5 * (L_in[n] + R_in[n])`.
- This keeps behavior deterministic and avoids channel-dependent widening.

## 4.2 Band split

- Implement LR4 crossover as cascaded Butterworth sections:
  - `xLow[n]` and `xHigh[n]`.
- Behavior:
  - `crossoverEnabled=true`: process only `xHigh` in Hilbert path, keep `xLow` mono.
  - `crossoverEnabled=false`: set `xLow=0`, `xHigh=x`.

## 4.3 Hilbert quadrature stage (IIR all-pass)

- Two all-pass branches produce near-quadrature outputs:
  - `I[n]` (in-phase reference branch).
  - `Q[n]` (quadrature branch).
- Use published coefficient sets (Niemitalo-style or equivalent) and verify in-repo with sweep tests.
- Target: phase delta around `90 deg +/- 5 deg` through most of `30 Hz..16 kHz`, with relaxed tolerance near the edges.

## 4.4 Width matrix and compensation

Let:

- `w = widthPct / 100`
- `gm = sqrt(1 - w)`
- `gq = sqrt(w)`
- `gComp = 1 / sqrt(1 - 0.5 * w)` (compensates expected average mono loss as width increases)

High-band stereo:

- `Lh[n] = gComp * (gm * xHigh[n] + gq * I[n])`
- `Rh[n] = gComp * (gm * xHigh[n] + gq * Q[n])`

With crossover enabled:

- `L[n] = xLow[n] + Lh[n]`
- `R[n] = xLow[n] + Rh[n]`

With crossover bypass:

- `L[n] = Lh[n]`
- `R[n] = Rh[n]`

Apply optional advanced transforms after this stage:

- `phaseAngleDeg` offsets quadrature relationship around `90 deg`.
- `phaseRotationDeg` rotates the final stereo vector.

Apply `outputGainDb` last.

## 5. Public Parameters / Interfaces

## 5.1 Parameter schema

- `crossoverEnabled` (bool, default `true`)
- `crossoverHz` (`20..500`, default `90`)
- `widthPct` (`0..100`, default `0`)
- `phaseAngleDeg` (`0..180`, default `90`, advanced)
- `phaseRotationDeg` (`-180..180`, default `0`, advanced)
- `outputGainDb` (`-60..+12`, default `0`)
  - UI may display `-inf` for values at floor.

## 5.2 DSP classes

- `BandSplitProcessor`
- `HilbertQuadratureProcessor`
- `StereoMatrixProcessor`

## 5.3 UI classes

- `GoniometerComponent`
- `CorrelationMeter`

## 6. Visualizer Design

## 6.1 XY mapping

Per rendered sample (or decimated sample):

- `x = (L - R) / sqrt(2)`  (horizontal, side)
- `y = (L + R) / sqrt(2)`  (vertical, mid)

Interpretation:

- Mono trends to vertical line (x near 0).
- Anti-phase trends to horizontal line.
- Ideal quadrature trends toward a circle.

## 6.2 Rendering policy

- Use a lock-free FIFO from audio thread to UI thread.
- Decimate points for stable frame time.
- Apply short persistence/fade to improve readability.

## 6.3 Correlation meter

Windowed Pearson-style estimate:

- `rho = sum(L*R) / sqrt(sum(L^2) * sum(R^2))`
- Clamp to `[-1, 1]`.
- Display as:
  - `+1`: mono-like
  - `0`: decorrelated/quadrature-like
  - `-1`: anti-phase

## 7. Acceptance Criteria

- DSP:
  - I/Q phase target near `90 deg +/- 5 deg` over most of `30 Hz..16 kHz`.
  - Bounded I/Q magnitude mismatch across sweep.
- Mono:
  - `width=0%`: fold-down equals mono baseline within `+/-0.1 dB`.
  - `width=100%`: `(L+R)/2` remains within `+/-1.0 dB` RMS for defined test material.
  - No narrow comb-style nulls deeper than `3 dB` in sweep-based fold-down analysis.
- Crossover:
  - Enabled mode keeps low band mono.
  - Bypass mode applies widening full-band.
- UI:
  - Goniometer orientation and correlation values match known synthetic cases.
- Packaging:
  - AU and VST3 targets build successfully.

## 8. Out of Scope

- FIR Hilbert mode and latency-switching UX.
- Standalone target.
- Marketing language claiming absolute mono compatibility.
