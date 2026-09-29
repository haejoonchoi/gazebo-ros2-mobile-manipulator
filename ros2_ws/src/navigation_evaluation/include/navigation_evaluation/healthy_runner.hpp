// Copyright 2026 Open Source Robotics Foundation, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include "navigation_evaluation/artifact_store.hpp"
#include "navigation_evaluation/scenario.hpp"

namespace navigation_evaluation {

enum class RunState
{
  Created,
  Starting,
  Ready,
  Running,
  Succeeded,
  Failed,
  Canceled,
  Interrupted
};

class HealthyRunner
{
public:
  HealthyRunner(
    std::filesystem::path root,
    std::string batch_id,
    std::string run_id,
    Scenario scenario,
    std::string profile_id);

  void start_run();
  void mark_ready();
  void mark_running();
  void finish_success(std::int64_t duration_ms);
  void finish_failure(const std::string &failure_class, std::int64_t duration_ms);
  void cancel(std::int64_t duration_ms);
  void interrupt(std::int64_t duration_ms);

  [[nodiscard]] RunState state() const noexcept { return state_; }
  [[nodiscard]] std::string terminal_status() const noexcept { return terminal_status_; }
  [[nodiscard]] const std::filesystem::path &run_path() const noexcept { return artifact_store_.run_path(); }

private:
  void append_state_event(const std::string &event_type, const std::string &state_name);

  ArtifactStore artifact_store_;
  Scenario scenario_;
  std::string run_id_;
  std::string profile_id_;
  std::string batch_id_;
  RunState state_{RunState::Created};
  std::string terminal_status_{"created"};
  std::uint64_t event_sequence_{0};
};

}  // namespace navigation_evaluation
