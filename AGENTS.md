# QuadraBass Agent Instructions

## Phase Boundaries
- Keep Phase 1 initialization documents authoritative for repository scaffolding:
  - `docs/plans/2026-02-24-quadrabass-init-design.md`
  - `docs/plans/2026-02-24-quadrabass-initialization.md`
- Treat all quadrature DSP and UI behavior as Phase 2 work.

## Spec Location
- Phase 2 design: `docs/plans/2026-02-24-quadrabass-phase2-design.md`
- Phase 2 implementation: `docs/plans/2026-02-24-quadrabass-phase2-implementation.md`
- Phase 2 completion guide: `docs/plans/2026-02-25-quadrabass-phase2-timeline.md`

## DSP Guardrails (Phase 2)
- Mono reference is average fold-down: `(L + R) / 2`.
- Crossover is Linkwitz-Riley, adjustable `20..500 Hz`, default `90 Hz`, enabled by default, with bypass/full-band mode.
- Hilbert implementation for v1 is low-latency IIR all-pass quadrature only; FIR mode is out of scope.
- `phaseAngleDeg` (`0..180`, default `90`) and `phaseRotationDeg` (`-180..180`, default `0`) are advanced controls only.
- Default width is `0%`.
- Plugin targets for Phase 2 are `AU` and `VST3` only.
- Use quantified compatibility language. Do not claim absolute "100% mono compatible" behavior.

## Mandatory Verification Before Completion
- Phase sweep checks:
  - I/Q phase delta near `90 deg +/- 5 deg` across most of `30 Hz..16 kHz` (relaxed edge-band tolerance).
  - Bounded I/Q magnitude mismatch across sweep.
- Mono fold-down checks:
  - `width=0%` matches mono baseline.
  - `width=100%` keeps `(L+R)/2` level within defined tolerance and avoids comb-style nulling.
- Crossover behavior checks:
  - low band remains mono when crossover is enabled.
  - full-band mode behaves correctly when crossover is bypassed.
- Visualizer checks:
  - mono trends vertical, quadrature trends circular, correlation meter responds correctly.
- Build gates:
  - `AU` and `VST3` builds must pass.
