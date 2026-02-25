# QuadraBass Phase 2 DSP Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Implement the production Phase 2 DSP and UI behavior for QuadraBass, including IIR quadrature widening, mono-safe crossover policy, advanced phase controls, and metering validation.

**Architecture:** Process a mono-derived source through an optional LR4 band split, apply high-band IIR Hilbert quadrature, and matrix output with a compensated width law targeting stable average mono fold-down `(L+R)/2`. Provide advanced phase controls and a goniometer + correlation meter for real-time feedback.

**Tech Stack:** C++17, JUCE DSP/APVTS, CMake, CTest, AU/VST3 targets.

---

## Delivery Guidance

- Execution rules and completion criteria are tracked in:
  - `docs/plans/2026-02-25-quadrabass-phase2-timeline.md`
- Follow that guide for sequencing, verification gates, and done criteria.

## Preconditions

- Work in feature branch/worktree (not `main`).
- Phase 1 initialization structure exists (CMake plugin target, src layout, tests, scripts).
- Existing Phase 1 docs remain unchanged:
  - `docs/plans/2026-02-24-quadrabass-init-design.md`
  - `docs/plans/2026-02-24-quadrabass-initialization.md`

## Skills Required During Execution

- `@test-driven-development` for all behavior changes.
- `@verification-before-completion` before any completion claims or integration steps.

### Task 1: Lock Plugin Formats and Parameter Contract

**Files:**
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/CMakeLists.txt`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/util/Params.h`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/util/Params.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/ParamLayoutTests.cpp`

**Step 1: Write failing test**

Add/extend tests asserting parameter IDs, ranges, and defaults:

- `crossoverEnabled` default true
- `crossoverHz` range `20..500`, default `90`
- `widthPct` range `0..100`, default `0`
- `phaseAngleDeg` range `0..180`, default `90`
- `phaseRotationDeg` range `-180..180`, default `0`
- `outputGainDb` range `-60..12`, default `0`

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R ParamLayout
```

Expected: failing assertions for missing or mismatched params.

**Step 3: Implement minimal code**

- Update APVTS schema in `Params`.
- Ensure AU/VST3 are the only enabled plugin formats in CMake for Phase 2.

**Step 4: Re-run test**

Run the same command; expected pass.

**Step 5: Commit**

```bash
git add CMakeLists.txt src/util/Params.* tests/ParamLayoutTests.cpp
git commit -m "feat: lock Phase 2 plugin formats and parameter contract"
```

### Task 2: Implement LR4 Band Split Processor

**Files:**
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/BandSplitProcessor.h`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/BandSplitProcessor.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/BandSplitProcessorTests.cpp`

**Step 1: Write failing test**

Add tests for:

- `crossoverEnabled=true` keeps low-band output mono.
- `crossoverEnabled=false` behaves as full-band high path.
- Recombination `low + high` approximates unity transfer.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R BandSplitProcessor
```

**Step 3: Implement minimal code**

- Add LR4 low-pass/high-pass processing per channel block.
- Support runtime updates for `crossoverHz` and enable/bypass state.

**Step 4: Re-run test**

Expected: all `BandSplitProcessor` tests pass.

**Step 5: Commit**

```bash
git add src/dsp/BandSplitProcessor.* tests/BandSplitProcessorTests.cpp
git commit -m "feat: implement LR4 split with mono low-band policy"
```

### Task 3: Implement IIR Hilbert Quadrature Processor

**Files:**
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/HilbertQuadratureProcessor.h`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/HilbertQuadratureProcessor.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/HilbertQuadratureTests.cpp`

**Step 1: Write failing test**

Add sweep-based tests for:

- I/Q phase delta near `90 deg +/- 5 deg` over most of `30 Hz..16 kHz`.
- Magnitude mismatch bounded (for example within `+/-1.5 dB`) over same region.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R HilbertQuadrature
```

**Step 3: Implement minimal code**

- Implement two-branch IIR all-pass network.
- Load published coefficient set and expose coefficient table as constants.
- Ensure sample-rate aware initialization/reset paths.

**Step 4: Re-run test**

Expected: phase/magnitude tests pass with relaxed edge-band tolerance.

**Step 5: Commit**

```bash
git add src/dsp/HilbertQuadratureProcessor.* tests/HilbertQuadratureTests.cpp
git commit -m "feat: add low-latency IIR Hilbert quadrature processor"
```

### Task 4: Implement Stereo Matrix With Width Compensation

**Files:**
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/StereoMatrixProcessor.h`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/StereoMatrixProcessor.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/StereoMatrixProcessorTests.cpp`

**Step 1: Write failing test**

Add tests for:

- `widthPct=0` produces mono-equivalent output.
- `widthPct=100` outputs quadrature mapping and keeps fold-down `(L+R)/2` within tolerance.
- `outputGainDb` applies final gain after matrix.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R StereoMatrixProcessor
```

**Step 3: Implement minimal code**

Use:

- `w = widthPct / 100`
- `gm = sqrt(1 - w)`
- `gq = sqrt(w)`
- `gComp = 1 / sqrt(1 - 0.5 * w)`

Then:

- `Lh = gComp * (gm * high + gq * I)`
- `Rh = gComp * (gm * high + gq * Q)`
- Add low-band mono component when crossover is enabled.
- Apply `outputGainDb` last.

**Step 4: Re-run test**

Expected: all matrix tests pass.

**Step 5: Commit**

```bash
git add src/dsp/StereoMatrixProcessor.* tests/StereoMatrixProcessorTests.cpp
git commit -m "feat: implement compensated quadrature stereo matrix"
```

### Task 5: Wire Processor Graph and Advanced Controls

**Files:**
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginProcessor.h`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginProcessor.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/PluginProcessorFlowTests.cpp`

**Step 1: Write failing test**

Add processor-level tests covering:

- Stereo input is internally mono-derived before widening.
- Crossover enable/bypass routing behavior.
- Advanced controls are applied safely and remain numerically stable.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R PluginProcessorFlow
```

**Step 3: Implement minimal code**

- Wire `BandSplitProcessor -> HilbertQuadratureProcessor -> StereoMatrixProcessor`.
- Apply `phaseAngleDeg` and `phaseRotationDeg` only in advanced path.
- Add denormal protection and safe reset handling.

**Step 4: Re-run test**

Expected: flow tests pass.

**Step 5: Commit**

```bash
git add src/PluginProcessor.* tests/PluginProcessorFlowTests.cpp
git commit -m "feat: wire Phase 2 DSP graph and advanced controls"
```

### Task 6: Implement Goniometer and Correlation Meter

**Files:**
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/GoniometerComponent.h`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/GoniometerComponent.cpp`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/CorrelationMeter.h`
- Create/Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/CorrelationMeter.cpp`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginEditor.h`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginEditor.cpp`
- Test: `/Users/meebs/dev/projects/personal/QuadraBass/tests/VisualizerMathTests.cpp`

**Step 1: Write failing test**

Add tests for:

- XY mapping: `x=(L-R)/sqrt(2)`, `y=(L+R)/sqrt(2)`.
- Correlation estimator returns expected values for mono, quadrature-like, anti-phase signals.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R VisualizerMath
```

**Step 3: Implement minimal code**

- Implement lock-free sample transfer from audio thread to UI.
- Render decimated/persistent XY trace.
- Add correlation meter widget and advanced controls panel.

**Step 4: Re-run test**

Expected: visualizer math tests pass.

**Step 5: Commit**

```bash
git add src/ui src/PluginEditor.* tests/VisualizerMathTests.cpp
git commit -m "feat: add goniometer and correlation metering"
```

### Task 7: End-to-End DSP Verification

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/tests/DspComplianceTests.cpp`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/CMakeLists.txt` (if needed for test registration)

**Step 1: Write failing tests**

Add end-to-end tests for:

- phase sweep tolerance across `30 Hz..16 kHz`.
- fold-down behavior at width `0%` and `100%`.
- no deep comb-style nulls in fold-down sweep analysis.
- low-band mono integrity when crossover is enabled.

**Step 2: Run test to verify failure**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure -R DspCompliance
```

**Step 3: Implement minimal code**

- Adjust coefficient set, smoothing, or compensation constants only as needed to satisfy acceptance tests.

**Step 4: Re-run test**

Expected: all compliance tests pass.

**Step 5: Commit**

```bash
git add tests/DspComplianceTests.cpp CMakeLists.txt src
git commit -m "test: add and satisfy Phase 2 DSP compliance suite"
```

### Task 8: Final Verification and Integration Readiness

**Files:**
- Modify only if failures are found during verification.

**Step 1: Run full test/build verification**

```bash
./scripts/format.sh check
./scripts/configure.sh -G Ninja -DBUILD_TESTING=ON -DZL_JUCE_COPY_PLUGIN=FALSE
./scripts/build.sh --config Release
ctest --test-dir build --output-on-failure
```

Expected: all checks pass, with AU/VST3 outputs configured.

**Step 2: Confirm requirements checklist**

Verify each locked decision is implemented:

- AU/VST3 only
- crossover policy and defaults
- advanced control gating
- quantified mono compatibility criteria
- visualizer/correlation behavior

**Step 3: Commit**

```bash
git add .
git commit -m "chore: finalize Phase 2 quadrature DSP implementation"
```
