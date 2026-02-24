# QuadraBass Repository Initialization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Initialize `QuadraBass` as a production-ready JUCE/CMake plugin repository with AGPL licensing, CI, scripts, tests, JUCE submodule, and GitHub remote `ethanmibu/QuadraBass`.

**Architecture:** Reuse proven `eq_infinity` conventions for project structure, scripts, and CI, while renaming and trimming to QuadraBass scope. Keep DSP/UI implementation at placeholder level with parameter wiring and safe pass-through behavior. Configure cross-platform plugin targets and a baseline test executable.

**Tech Stack:** C++17, JUCE (submodule), CMake 3.21+, CTest, GitHub Actions, clang-format.

---

## Preconditions

- Working directory: `/Users/meebs/dev/projects/personal/QuadraBass`
- Git initialized with design docs commit already present.
- Tools expected: `git`, `cmake`, `clang-format`, `gh` (for GitHub repo creation), shell `bash`.

## Skills To Apply During Execution

- `@test-driven-development` for behavior-bearing code additions.
- `@verification-before-completion` before any completion claims or final push.

### Task 1: Repository Hygiene and Metadata

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.gitignore`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.editorconfig`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.clang-format`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/LICENSE`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/README.md`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/AGENTS.md`

**Step 1: Write a failing documentation check (manual)**

Define expected files and fail if any are missing:

```bash
test -f .gitignore && test -f .editorconfig && test -f .clang-format && test -f LICENSE && test -f README.md && test -f AGENTS.md
```

**Step 2: Run check to verify it fails**

Run:

```bash
test -f .gitignore && test -f .editorconfig && test -f .clang-format && test -f LICENSE && test -f README.md && test -f AGENTS.md
```

Expected: non-zero exit because files do not exist yet.

**Step 3: Create minimal implementation**

- Add files with baseline content copied/adapted from `eq_infinity`:
  - `.gitignore` for OS/IDE/CMake/JUCE artifacts
  - `.editorconfig` for UTF-8/LF/indent style
  - `.clang-format` for C++ style consistency
  - `LICENSE` as AGPL-3.0 text
  - `README.md` with build, test, and project overview
  - `AGENTS.md` minimal project instructions

**Step 4: Run check to verify it passes**

Re-run the same `test -f ...` command.
Expected: zero exit status.

**Step 5: Commit**

```bash
git add .gitignore .editorconfig .clang-format LICENSE README.md AGENTS.md
git commit -m "chore: add repository metadata and hygiene files"
```

### Task 2: Add Build and Utility Scripts

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/scripts/configure.sh`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/scripts/build.sh`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/scripts/format.sh`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/scripts/setup_macos.sh`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/scripts/setup_windows.ps1`

**Step 1: Write failing script checks**

```bash
test -x scripts/configure.sh && test -x scripts/build.sh && test -x scripts/format.sh
```

**Step 2: Run check to verify it fails**

Run command above.
Expected: non-zero exit.

**Step 3: Write minimal implementation**

- Add scripts mirroring `eq_infinity` behavior with project names updated:
  - `configure.sh`: `cmake -S ... -B ...`
  - `build.sh`: `cmake --build ...`
  - `format.sh`: format/check modes over `src/` and `tests/`
  - setup scripts install/verify prerequisites per OS
- Set executable bit on shell scripts.

**Step 4: Run check to verify it passes**

```bash
test -x scripts/configure.sh && test -x scripts/build.sh && test -x scripts/format.sh
```

Expected: zero exit.

**Step 5: Commit**

```bash
git add scripts
git commit -m "build: add configure build format and setup scripts"
```

### Task 3: Create JUCE/CMake Plugin Skeleton

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/CMakeLists.txt`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginProcessor.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginProcessor.cpp`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginEditor.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginEditor.cpp`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/util/Params.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/util/Params.cpp`

**Step 1: Write the failing configure test**

```bash
./scripts/configure.sh -G Ninja -DZL_JUCE_COPY_PLUGIN=FALSE -DBUILD_TESTING=ON
```

**Step 2: Run test to verify it fails**

Run command above.
Expected: configure fails because no CMake project files exist.

**Step 3: Write minimal implementation**

- Add `CMakeLists.txt` using JUCE submodule path `third_party/JUCE`.
- Configure plugin target `QuadraBass` and formats by platform.
- Add APVTS parameter schema for:
  - Crossover (20-500 Hz)
  - Width (0-100%)
  - Phase Angle (0-180 deg, default 90)
  - Phase Rotation (-180 to 180 deg)
  - Gain (-60 to +12 dB or equivalent floor for practical -inf display)
- Implement pass-through-safe processor/editor skeleton.

**Step 4: Run configure to verify it passes (or reaches JUCE missing check)**

```bash
./scripts/configure.sh -G Ninja -DZL_JUCE_COPY_PLUGIN=FALSE -DBUILD_TESTING=ON
```

Expected: if JUCE not yet added, explicit error `JUCE not found at third_party/JUCE`; after Task 6 this command will pass.

**Step 5: Commit**

```bash
git add CMakeLists.txt src
git commit -m "feat: add QuadraBass plugin skeleton and parameter layout"
```

### Task 4: Add DSP and UI Placeholder Components

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/BandSplitProcessor.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/BandSplitProcessor.cpp`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/HilbertProcessor.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/HilbertProcessor.cpp`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/StereoMatrixProcessor.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/dsp/StereoMatrixProcessor.cpp`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/StereometerComponent.h`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/src/ui/StereometerComponent.cpp`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginProcessor.cpp`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/src/PluginEditor.cpp`

**Step 1: Write failing compile expectation**

Build should fail until new placeholders are wired:

```bash
./scripts/build.sh --config Release
```

**Step 2: Run build to verify it fails**

Run command above (after configure and JUCE submodule are in place).
Expected: compile/link errors due to missing DSP/UI symbols.

**Step 3: Write minimal implementation**

- Implement lightweight class APIs and no-op/pass-through process methods.
- Wire processor to call placeholder chain in `processBlock` safely.
- Add basic UI region for a future stereometer and control placeholders.

**Step 4: Run build to verify it passes**

```bash
./scripts/build.sh --config Release
```

Expected: successful build of plugin targets.

**Step 5: Commit**

```bash
git add src
git commit -m "feat: add DSP and stereometer placeholders with safe wiring"
```

### Task 5: Add Test Harness

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/tests/InitTests.cpp`
- Modify: `/Users/meebs/dev/projects/personal/QuadraBass/CMakeLists.txt`

**Step 1: Write failing test**

Add test asserting parameter layout includes all required IDs and defaults.

```cpp
expect (layoutContains ("crossover"));
expect (layoutContains ("width"));
expect (layoutContains ("phaseAngle"));
expect (layoutContains ("phaseRotation"));
expect (layoutContains ("outputGain"));
```

**Step 2: Run test target to verify it fails**

```bash
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure
```

Expected: initial failure until CMake target and/or assertions are complete.

**Step 3: Write minimal implementation**

- Add `quadrabass_tests` executable and CTest registration.
- Ensure test links required JUCE modules and includes `Params` code.
- Fix parameter IDs/defaults in source as needed.

**Step 4: Run tests to verify they pass**

```bash
./scripts/configure.sh -G Ninja -DBUILD_TESTING=ON -DZL_JUCE_COPY_PLUGIN=FALSE
./scripts/build.sh --target quadrabass_tests --config Release
ctest --test-dir build --output-on-failure
```

Expected: all tests pass.

**Step 5: Commit**

```bash
git add CMakeLists.txt tests src/util/Params.*
git commit -m "test: add baseline parameter and initialization tests"
```

### Task 6: Add JUCE Submodule and Verify Configure

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.gitmodules`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/third_party/JUCE` (git submodule)

**Step 1: Write failing submodule check**

```bash
test -f third_party/JUCE/CMakeLists.txt
```

**Step 2: Run check to verify it fails**

Run command above.
Expected: non-zero exit before submodule add/init.

**Step 3: Write minimal implementation**

```bash
git submodule add https://github.com/juce-framework/JUCE.git third_party/JUCE
git submodule update --init --recursive
```

**Step 4: Run checks and configure**

```bash
test -f third_party/JUCE/CMakeLists.txt
./scripts/configure.sh -G Ninja -DBUILD_TESTING=ON -DZL_JUCE_COPY_PLUGIN=FALSE
```

Expected: submodule file exists; configure succeeds.

**Step 5: Commit**

```bash
git add .gitmodules third_party/JUCE
git commit -m "build: add JUCE as git submodule"
```

### Task 7: Add GitHub Actions CI

**Files:**
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.github/workflows/build-macos.yml`
- Create: `/Users/meebs/dev/projects/personal/QuadraBass/.github/workflows/build-windows.yml`

**Step 1: Write failing workflow existence check**

```bash
test -f .github/workflows/build-macos.yml && test -f .github/workflows/build-windows.yml
```

**Step 2: Run check to verify it fails**

Run command above.
Expected: non-zero exit.

**Step 3: Write minimal implementation**

- Add macOS workflow with:
  - checkout + recursive submodules
  - cmake setup
  - formatting check
  - configure/build/test
- Add Windows workflow with:
  - checkout + recursive submodules
  - cmake setup
  - configure/build/test using bash shell

**Step 4: Run local YAML sanity check**

```bash
test -s .github/workflows/build-macos.yml && test -s .github/workflows/build-windows.yml
```

Expected: zero exit and non-empty files.

**Step 5: Commit**

```bash
git add .github/workflows
git commit -m "ci: add macOS and Windows build pipelines"
```

### Task 8: Run End-to-End Verification Before Publishing

**Files:**
- Modify (if needed): any files discovered during verification.

**Step 1: Run formatting check**

```bash
./scripts/format.sh check
```

Expected: formatting check passes.

**Step 2: Run full configure/build/tests**

```bash
./scripts/configure.sh -G Ninja -DBUILD_TESTING=ON -DZL_JUCE_COPY_PLUGIN=FALSE
./scripts/build.sh --config Release
ctest --test-dir build --output-on-failure
```

Expected: all commands succeed; tests pass.

**Step 3: Fix only issues that fail verification**

- Apply minimal corrections required by failing checks.

**Step 4: Re-run full verification**

Run Step 1 and Step 2 again.
Expected: clean pass.

**Step 5: Commit**

```bash
git add .
git commit -m "chore: finalize verified QuadraBass initialization"
```

### Task 9: Create/Link GitHub Remote and Push

**Files:**
- No file creation required; git remote state changes.

**Step 1: Write failing remote check**

```bash
git remote get-url origin
```

**Step 2: Run check to verify it fails**

Run command above.
Expected: error `No such remote 'origin'`.

**Step 3: Create and link repository**

```bash
gh repo create ethanmibu/QuadraBass --public --source=. --remote=origin --push
```

If repo already exists:

```bash
git remote add origin git@github.com:ethanmibu/QuadraBass.git
git push -u origin main
```

**Step 4: Verify remote linkage**

```bash
git remote -v
```

Expected: `origin` points to `ethanmibu/QuadraBass` for fetch/push.

**Step 5: Commit**

No additional commit required unless Step 3 or Step 4 surfaced file changes.

