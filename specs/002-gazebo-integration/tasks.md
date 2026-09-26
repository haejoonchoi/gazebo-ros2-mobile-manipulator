# Tasks: Gazebo Integration for Constrained Hardware

**Input**: Design documents from `/specs/002-gazebo-integration/`

**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`

**Tests**: Required by the feature specification and constitution. For each behavior, add the test first, confirm it fails, then implement.

**Organization**: Tasks are grouped by user story so each increment can be demonstrated and validated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Safe to execute in parallel because files and prerequisites do not conflict.
- **[USn]**: Maps the task to a user story in `spec.md`.

## Phase 1: Setup

- [ ] T001 Record final Gazebo Harmonic + ROS 2 Jazzy package/version compatibility and host assumptions in `specs/002-gazebo-integration/research.md`
- [ ] T002 Add/update low-resource execution profiles and defaults in `ros2_ws/src/robot_sim_bringup/config/`
- [ ] T003 [P] Add/update dependency and setup notes for Gazebo-only workflow in `specs/002-gazebo-integration/quickstart.md`
- [ ] T004 [P] Add/update architecture and contract references for Gazebo simulator scope in `specs/002-gazebo-integration/contracts/ros-interfaces.md`

## Phase 2: Foundational (Blocking Prerequisites)

- [ ] T005 [P] Add failing launch contract tests for `/clock`, TF connectivity, lidar, odometry, and bridge readiness in `ros2_ws/src/robot_sim_bringup/test/`
- [ ] T006 [P] Add failing readiness tests for bounded simulator startup and positive clock progress in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T007 Implement Gazebo bringup launch wiring for server-only and optional graphical mode in `ros2_ws/src/robot_sim_bringup/launch/`
- [ ] T008 Implement readiness checks for transforms, sensor freshness, services, and clock-domain validation in `ros2_ws/src/navigation_evaluation/src/`
- [ ] T009 Add scenario schema coverage for execution profile linkage and metadata identity in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T010 Implement scenario/execution-profile validation updates in `ros2_ws/src/navigation_evaluation/src/`

**Checkpoint**: Gazebo startup, bridge contracts, and bounded readiness are testable before user-story implementation.

## Phase 3: User Story 1 - Run a Healthy Navigation Scenario on Gazebo (Priority: P1) MVP

**Goal**: Run healthy single and batch navigation tasks in server-only mode with complete artifacts.

**Independent Test**: Execute one healthy run and one 20-trial batch with complete terminal classifications.

- [ ] T011 [P] [US1] Add failing lifecycle and terminal-classification tests for healthy run flow in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T012 [P] [US1] Add failing end-to-end healthy launch test for one complete run artifact in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T013 [US1] Implement Gazebo-backed scenario runner readiness-to-terminal flow in `ros2_ws/src/navigation_evaluation/src/scenario_runner_node.cpp`
- [ ] T014 [US1] Implement run metrics/event collection including clock domain, simulator identity, and execution mode in `ros2_ws/src/navigation_evaluation/src/metrics_collector_node.cpp`
- [ ] T015 [US1] Implement batch orchestration with exact-count terminal outcomes in `ros2_ws/src/navigation_evaluation/src/run_batch.cpp`
- [ ] T016 [US1] Add `healthy.yaml` scenario and launch wiring in `ros2_ws/src/navigation_evaluation/config/scenarios/` and `ros2_ws/src/robot_sim_bringup/launch/`
- [ ] T017 [US1] Capture one validated healthy-run fixture and document baseline limitations in `docs/reproducibility.md`

**Checkpoint**: User Story 1 is independently demoable and is the MVP.

## Phase 4: User Story 2 - Measure Repeatability and Resource Use (Priority: P2)

**Goal**: Ensure repeatability reporting and bounded termination under constrained resources.

**Independent Test**: Run 20 healthy trials and verify statistics plus resource-limit classification behavior.

- [ ] T018 [P] [US2] Add failing tests for resource-threshold observations and classified termination behavior in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T019 [P] [US2] Add failing tests for run-set comparability metadata (world, robot, scenario hash, host assumptions) in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T020 [US2] Implement resource observation capture and classification mapping in `ros2_ws/src/navigation_evaluation/src/`
- [ ] T021 [US2] Implement comparability metadata persistence in run artifacts and summaries in `ros2_ws/src/navigation_evaluation/src/artifact_store.cpp`
- [ ] T022 [US2] Execute and record 20-trial healthy batch evidence in `docs/results.md`

**Checkpoint**: Repeatability and resource-aware outcomes are demonstrable with evidence.

## Phase 5: User Story 3 - Exercise Faults and Restart Safely (Priority: P3)

**Goal**: Inject lidar delay/outage faults and verify clean restart isolation.

**Independent Test**: Run delay and outage scenarios, then run healthy scenario after restart and verify run-state isolation.

- [ ] T023 [P] [US3] Add failing delay and outage policy tests (timing, ordering, and bounded behavior) in `ros2_ws/src/sensor_fault_injection/test/`
- [ ] T024 [P] [US3] Add failing requested-vs-observed fault event contract tests in `ros2_ws/src/sensor_fault_injection/test/`
- [ ] T025 [US3] Implement delay and outage policies in `ros2_ws/src/sensor_fault_injection/src/fault_policy.cpp`
- [ ] T026 [US3] Implement raw-to-impaired lidar relay and fault event publication in `ros2_ws/src/sensor_fault_injection/src/lidar_fault_injector_node.cpp`
- [ ] T027 [US3] Route Nav2 through impaired lidar topic while preserving raw stream in `ros2_ws/src/robot_sim_bringup/config/`
- [ ] T028 [US3] Add `lidar_delay.yaml` and `lidar_outage.yaml` scenario definitions in `ros2_ws/src/navigation_evaluation/config/scenarios/`
- [ ] T029 [US3] Add failing restart/isolation tests for canceled, failed, and interrupted runs in `ros2_ws/src/navigation_evaluation/test/`
- [ ] T030 [US3] Implement bounded reset flow (delete/reset/respawn/relocalize) and stale-state prevention in `ros2_ws/src/navigation_evaluation/src/`

**Checkpoint**: Fault injection and restart isolation pass independent scenario tests.

## Phase 6: User Story 4 - Diagnose and Compare Gazebo Runs (Priority: P4)

**Goal**: Provide responsive live diagnostics and accurate run-set comparison.

**Independent Test**: Observe live run changes and compare two compatible run sets; open an incomplete artifact safely.

- [ ] T031 [P] [US4] Add failing artifact compatibility and comparison-model tests in `ros2_ws/src/diagnostics_ui/test/`
- [ ] T032 [P] [US4] Add failing ROS worker and update-rate tests (>=2 Hz UI, 20 Hz synthetic input) in `ros2_ws/src/diagnostics_ui/test/`
- [ ] T033 [US4] Implement artifact reader and run-set comparison model in `ros2_ws/src/diagnostics_ui/src/run_comparison.cpp`
- [ ] T034 [US4] Implement off-GUI-thread ROS worker snapshots for nav state, freshness, fault state, and clock status in `ros2_ws/src/diagnostics_ui/src/ros_worker.cpp`
- [ ] T035 [US4] Implement live diagnostics and comparison views in `ros2_ws/src/diagnostics_ui/src/main_window.cpp`
- [ ] T036 [US4] Add incomplete/unsupported artifact handling coverage in `ros2_ws/src/diagnostics_ui/test/`

**Checkpoint**: UI diagnostics and comparison workflow are independently demonstrable.

## Phase 7: Evidence and Cross-Cutting Quality

- [ ] T037 [P] Update quickstart verification steps from clean-host execution in `specs/002-gazebo-integration/quickstart.md`
- [ ] T038 [P] Add/update architecture and TF diagrams in `docs/architecture.md` and `docs/frames.md`
- [ ] T039 [P] Add evidence-based debugging narrative for one captured failure/recovery case in `docs/debugging.md`
- [ ] T040 [P] Ensure validation scripts cover unit, launch, scenario, and restart checks in `scripts/`
- [ ] T041 Run Spec Kit analysis/convergence and resolve remaining acceptance gaps under `specs/002-gazebo-integration/`
- [ ] T042 Audit published results for trial counts, metadata completeness, and limitations language in `docs/results.md`

## Dependencies & Execution Order

- Phase 1 precedes foundational work.
- Phase 2 blocks all user stories.
- User Story 1 is MVP and should be completed first.
- User Story 2 depends on User Story 1 run artifacts and batch pipeline.
- User Story 3 depends on User Story 1 runner/metrics and simulator reset behavior.
- User Story 4 depends on stable artifacts from User Stories 1-3.
- Phase 7 starts as stories stabilize, then completes after all target stories.

## Daily 1-2 Hour Execution Strategy

- Work one unchecked task per session.
- Prefer tasks that are independently testable and finishable in one session.
- If a task exceeds one session, split it into two smaller tasks before continuing.
- End each session by running the smallest relevant test scope and checking off completed tasks.
