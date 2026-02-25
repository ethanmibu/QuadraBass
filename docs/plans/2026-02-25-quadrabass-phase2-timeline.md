# QuadraBass Phase 2 Completion Guide

**Date:** 2026-02-25  
**Project:** QuadraBass  
**Scope:** Non-chronological execution rules, instructions, and completion guidance for Phase 2

## 1. Source-of-Truth Documents

- Design and technical constraints:
  - `docs/plans/2026-02-24-quadrabass-phase2-design.md`
- Task-by-task implementation plan:
  - `docs/plans/2026-02-24-quadrabass-phase2-implementation.md`
- Project-wide guardrails:
  - `AGENTS.md`
- Ongoing execution notes:
  - `docs/plans/2026-02-24-quadrabass-phase2-execution-log.md` (if present in the branch)

If documents conflict, apply this order:
1. `AGENTS.md`
2. Phase 2 design doc
3. Phase 2 implementation plan
4. Execution log

## 2. Non-Negotiable Phase 2 Rules

- Build formats must remain AU/VST3 only.
- Mono reference and compatibility checks must use `(L + R) / 2`.
- Hilbert implementation for v1 remains IIR all-pass only.
- Advanced phase controls (`phaseAngleDeg`, `phaseRotationDeg`) must stay gated as advanced behavior.
- Default width must remain `0%`.
- No absolute "100% mono compatible" claims in docs, UI copy, or PR summaries.

## 3. Required Execution Order

Execute remaining work in the order below unless a documented dependency requires adjustment:
1. Task 4: Stereo matrix + width compensation.
2. Task 5: Processor graph wiring + advanced controls.
3. Task 6: Goniometer + correlation meter.
4. Task 7: DSP compliance suite.
5. Task 8: Final verification + integration readiness.

Parallel work is allowed only when it does not violate dependencies:
- UI visualization implementation can overlap with DSP work once processor APIs are stable.
- Compliance criteria and acceptance tests cannot be finalized until core DSP path (Tasks 4 and 5) is stable.

## 4. Task Completion Protocol

For each task:
1. Write failing tests for the task scope.
2. Run tests and verify expected failures.
3. Implement the minimal behavior needed to pass.
4. Re-run targeted tests.
5. Commit task-scoped changes with clear commit messages.
6. Update execution log with:
   - task status
   - files changed
   - commands run
   - outcomes

Do not continue to the next task until task-level verification passes.

## 5. Verification Gates

## 5.1 Task-level checks

- Run targeted task suites during implementation:
  - `ctest --test-dir build --output-on-failure -R <TaskRegex>`

## 5.2 Phase-level checks

Run before claiming Phase 2 completion:
- `./scripts/format.sh check`
- `./scripts/configure.sh -G Ninja -DBUILD_TESTING=ON -DZL_JUCE_COPY_PLUGIN=FALSE`
- `./scripts/build.sh --config Release`
- `ctest --test-dir build --output-on-failure`

All commands must pass on current branch head.

## 6. Documentation Rules During Development

When behavior changes:
- Update design doc if architecture/acceptance criteria changed.
- Update implementation plan if task order or scope changed.
- Update execution log after each completed task.
- Keep `AGENTS.md` consistent with any new hard guardrails.

Documentation updates are part of the task, not follow-up work.

## 7. Definition of Done (Phase 2)

Phase 2 is complete only when all are true:
- Tasks 4 through 8 are complete and verified.
- Phase and fold-down compliance criteria pass within defined tolerances.
- Goniometer/correlation behavior is validated by tests.
- AU/VST3-only policy is enforced in build configuration and docs.
- Full verification command set passes with no unresolved failures.
- Docs reflect the final shipped behavior.

## 8. Integration Readiness Checklist

- [ ] Branch contains only intentional Phase 2 changes.
- [ ] Verification results are current and recorded.
- [ ] Execution log is up to date through Task 8.
- [ ] No standalone references remain in project planning/build policy.
- [ ] Merge request/PR summary includes exact verification commands and outcomes.
