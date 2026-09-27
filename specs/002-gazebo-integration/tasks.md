# Tasks: Gazebo Integration for Constrained Hardware

**Input**: Design documents from `/specs/002-gazebo-integration/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Establish the Gazebo/Harmonic workspace baseline and shared execution assumptions.

- [ ] T001 Create the required ROS package layout and verify the active package boundaries in `ros2_ws/src/robot_sim_bringup/`, `ros2_ws/src/navigation_evaluation/`, `ros2_ws/src/sensor_fault_injection/`, and `ros2_ws/src/diagnostics_ui/`
- [ ] T002 [P] Update package manifests for Gazebo Harmonic, ROS 2 Jazzy, Nav2, and Qt dependencies in `ros2_ws/src/robot_sim_bringup/package.xml`, `ros2_ws/src/navigation_evaluation/package.xml`, `ros2_ws/src/sensor_fault_injection/package.xml`, and `ros2_ws/src/diagnostics_ui/package.xml`
- [ ] T003 [P] Capture the supported low-resource execution profile and setup commands in `specs/002-gazebo-integration/quickstart.md` and `specs/002-gazebo-integration/research.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Establish the core Gazebo world, bridge, readiness, and metrics contracts before any story-specific implementation.

- [ ] T004 Define the Gazebo execution profile schema and validation rules in `ros2_ws/src/navigation_evaluation/config/scenarios/` and `ros2_ws/src/robot_sim_bringup/config/`
- [ ] T005 [P] Create the low-complexity robot model, sensor frames, and world assets in `ros2_ws/src/robot_sim_bringup/urdf/` and `ros2_ws/src/robot_sim_bringup/worlds/`
- [ ] T006 [P] Add the headless server-only launch and `ros_gz_bridge` YAML configuration in `ros2_ws/src/robot_sim_bringup/launch/` and `ros2_ws/src/robot_sim_bringup/config/ros_gz_bridge.yaml`
- [ ] T007 Implement bounded readiness checks for `/clock`, TF connectivity, localization, sensor freshness, and Nav2 lifecycle in `ros2_ws/src/navigation_evaluation/src/ready_check.cpp`
- [ ] T008 Add run-level metadata and artifact versioning for the Gazebo execution profile in `ros2_ws/src/navigation_evaluation/include/navigation_evaluation/` and `ros2_ws/src/navigation_evaluation/src/`

**Checkpoint**: Foundation ready - Gazebo launch, clock, TF, sensor, and artifact contracts are available before user story work begins.

---

## Phase 3: User Story 1 - Run a Healthy Navigation Scenario on Gazebo (Priority: P1) 🎯 MVP

**Goal**: Bring up a supported Gazebo simulation and complete one healthy navigation run without Isaac Sim.

**Independent Test**: On a supported Ubuntu 24.04 workstation, launch the documented headless profile, execute one healthy scenario, and confirm that the robot reaches the goal and records a valid terminal artifact.

### Tests for User Story 1

- [ ] T009 [P] [US1] Write a healthy launch smoke test in `ros2_ws/src/navigation_evaluation/test/test_healthy_gazebo_launch.py`
- [ ] T010 [P] [US1] Write a single-run navigation contract test in `ros2_ws/src/navigation_evaluation/test/test_healthy_run.py`

### Implementation for User Story 1

- [ ] T011 [P] [US1] Create the Gazebo robot, lidar, and IMU model wiring in `ros2_ws/src/robot_sim_bringup/urdf/` and `ros2_ws/src/robot_sim_bringup/models/`
- [ ] T012 [P] [US1] Implement the bridged `/clock`, `/odom`, `/scan_raw`, `/imu/data`, and TF topics in `ros2_ws/src/robot_sim_bringup/config/ros_gz_bridge.yaml`
- [ ] T013 [US1] Implement the headless Gazebo bring-up and world spawn flow in `ros2_ws/src/robot_sim_bringup/launch/gazebo_headless.launch.py`
- [ ] T014 [US1] Implement the healthy scenario runner and task lifecycle from start to terminal status in `ros2_ws/src/navigation_evaluation/src/healthy_runner.cpp`
- [ ] T015 [US1] Persist one complete run artifact with scenario id, execution profile, resource observations, and terminal record in `ros2_ws/src/navigation_evaluation/src/artifact_store.cpp`

**Checkpoint**: User Story 1 should be fully functional and independently testable in headless mode.

---

## Phase 4: User Story 2 - Measure Repeatability and Resource Use (Priority: P2)

**Goal**: Execute repeatable 20-trial batches and record host/resource observations, timing, and success-rate statistics.

**Independent Test**: Run a 20-trial healthy batch, inspect the summary output, and confirm that the report contains exactly 20 terminal classifications and aggregate duration statistics.

### Tests for User Story 2

- [ ] T016 [P] [US2] Write a 20-trial batch validation test in `ros2_ws/src/navigation_evaluation/test/test_batch_summary.py`
- [ ] T017 [P] [US2] Write a constrained-resource smoke test in `ros2_ws/src/navigation_evaluation/test/test_resource_limit.py`

### Implementation for User Story 2

- [ ] T018 [P] [US2] Create the batch scenario profile and input manifest in `ros2_ws/src/navigation_evaluation/config/scenarios/healthy_batch.yaml`
- [ ] T019 [US2] Implement the batch runner, trial loop, and terminal classification logic in `ros2_ws/src/navigation_evaluation/src/batch_runner.cpp`
- [ ] T020 [US2] Collect CPU, memory, clock-progress, and host metadata into each run record in `ros2_ws/src/navigation_evaluation/src/resource_observer.cpp`
- [ ] T021 [US2] Compute success rate, median duration, and aggregate statistics for a run set in `ros2_ws/src/navigation_evaluation/src/run_set_summary.cpp`

**Checkpoint**: User Story 2 should independently verify repeatability and resource fidelity for the Gazebo baseline.

---

## Phase 5: User Story 3 - Exercise Faults and Restart Safely (Priority: P3)

**Goal**: Inject controlled lidar delay and outage while preserving raw data, then reset cleanly and isolate run state.

**Independent Test**: Run one lidar-delay scenario and one lidar-outage scenario, then start a healthy run after interruption and confirm that no stale run-id or fault state leaks into the new run.

### Tests for User Story 3

- [ ] T022 [P] [US3] Write a fault scheduling and raw-vs-impaired lidar test in `ros2_ws/src/sensor_fault_injection/test/test_fault_injection.py`
- [ ] T023 [P] [US3] Write a restart and state-isolation test in `ros2_ws/src/navigation_evaluation/test/test_restart_isolation.py`

### Implementation for User Story 3

- [ ] T024 [P] [US3] Implement the raw lidar relay and impaired-topic pass-through in `ros2_ws/src/sensor_fault_injection/src/lidar_relay.cpp`
- [ ] T025 [US3] Add delay and loss schedule support with requested-versus-observed event recording in `ros2_ws/src/sensor_fault_injection/src/fault_schedule.cpp`
- [ ] T026 [US3] Implement bounded reset flow for delete, respawn, localization reset, and costmap clearing in `ros2_ws/src/navigation_evaluation/src/reset_manager.cpp`
- [ ] T027 [US3] Ensure unique run ids, fresh event streams, and no stale state after a failed or interrupted trial in `ros2_ws/src/navigation_evaluation/src/run_manager.cpp`

**Checkpoint**: User Story 3 should independently validate fault injection, termination, and clean reset semantics.

---

## Phase 6: User Story 4 - Diagnose and Compare Gazebo Runs (Priority: P4)

**Goal**: Provide a live diagnostic interface and comparison view that can inspect current, partial, and completed run artifacts without manual parsing.

**Independent Test**: Open the Qt diagnostic interface during an active run and compare two completed run sets while opening an incomplete artifact, verifying that unsupported or partial data is surfaced clearly.

### Tests for User Story 4

- [ ] T028 [P] [US4] Write a Qt snapshot refresh test in `ros2_ws/src/diagnostics_ui/test/test_diagnostics_snapshot.cpp`
- [ ] T029 [P] [US4] Write a run-comparison and partial-artifact validation test in `ros2_ws/src/diagnostics_ui/test/test_compare_view.cpp`

### Implementation for User Story 4

- [ ] T030 [P] [US4] Implement the ROS diagnostic worker thread and immutable snapshot model in `ros2_ws/src/diagnostics_ui/src/diagnostic_worker.cpp`
- [ ] T031 [US4] Add live navigation, fault, and resource-state widgets in `ros2_ws/src/diagnostics_ui/src/`
- [ ] T032 [US4] Add compare-view logic for success rate, median duration, and sensor timing summaries in `ros2_ws/src/diagnostics_ui/src/compare_view.cpp`
- [ ] T033 [US4] Add partial-data and incompatible-schema detection for artifact loading in `ros2_ws/src/diagnostics_ui/src/artifact_loader.cpp`

**Checkpoint**: User Story 4 should independently validate live diagnostics and cross-run comparison behavior.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Final validation, documentation, and repo-wide cleanup for the Gazebo migration.

- [ ] T034 [P] Update architecture, resource budget, and debugging evidence in `docs/`, `specs/002-gazebo-integration/quickstart.md`, and `specs/002-gazebo-integration/research.md`
- [ ] T035 [P] Run the constrained-hardware smoke matrix and record the evidence for the headless and graphical profiles in `specs/002-gazebo-integration/quickstart.md`
- [ ] T036 Remove remaining Isaac Sim assumptions and ensure the Gazebo path is the only supported runtime in the workspace documentation and build configs
- [ ] T037 Validate final task and artifact contracts against `specs/002-gazebo-integration/contracts/ros-interfaces.md` and `specs/002-gazebo-integration/contracts/artifact-schema.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion and blocks all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational completion
- **User Story 2 (Phase 4)**: Depends on Foundational completion and integrates with US1 outputs
- **User Story 3 (Phase 5)**: Depends on US1 and core Gazebo readiness
- **User Story 4 (Phase 6)**: Depends on US1 and US2/US3 artifact contracts
- **Polish (Phase 7)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: No dependencies on other stories; this is the MVP
- **User Story 2 (P2)**: Builds on US1 run records and artifact schema
- **User Story 3 (P3)**: Builds on US1 readiness and reset contracts
- **User Story 4 (P4)**: Builds on US1-US3 execution and artifact outputs

### Parallel Opportunities

- `T002`, `T003` can run in parallel during Setup
- `T005`, `T006`, `T007`, `T008` can run in parallel once Setup is complete
- `T009`, `T010` can run in parallel within User Story 1
- `T011`, `T012` can run in parallel within User Story 1
- `T016`, `T017` can run in parallel within User Story 2
- `T022`, `T023` can run in parallel within User Story 3
- `T028`, `T029` can run in parallel within User Story 4
- `T034`, `T035` can run in parallel during final validation

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 and Phase 2
2. Complete User Story 1
3. Validate with the headless healthy scenario and artifact contract
4. Stop and confirm the Gazebo baseline is stable before continuing

### Incremental Delivery

1. Setup + Foundational → baseline Gazebo launch and readiness
2. User Story 1 → healthy single-run execution
3. User Story 2 → batch repeatability and resource measurement
4. User Story 3 → controlled faults and restart safety
5. User Story 4 → diagnostics and comparison view
6. Polish → documentation, smoke validation, and cleanup

## Notes

- `[P]` tasks are parallelizable because they touch different files or independent test scopes
- Each phase is intentionally independent and testable
- All tasks use explicit repository file paths so an LLM can execute the work without further context
- The task list is scoped to the active Gazebo migration and excludes Isaac Sim from the required implementation path
