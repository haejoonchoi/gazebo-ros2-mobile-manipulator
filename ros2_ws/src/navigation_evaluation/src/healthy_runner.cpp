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
#include "navigation_evaluation/healthy_runner.hpp"

#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <thread>

#include <sys/utsname.h>
#include <unistd.h>

namespace navigation_evaluation {
namespace {

double timestamp_seconds()
{
  using namespace std::chrono;
  const auto now = system_clock::now().time_since_epoch();
  return duration_cast<duration<double>>(now).count();
}

std::string state_name(RunState state)
{
  switch (state) {
    case RunState::Created:
      return "created";
    case RunState::Starting:
      return "starting";
    case RunState::Ready:
      return "ready";
    case RunState::Running:
      return "running";
    case RunState::Succeeded:
      return "succeeded";
    case RunState::Failed:
      return "failed";
    case RunState::Canceled:
      return "canceled";
    case RunState::Interrupted:
      return "interrupted";
  }
  return "unknown";
}

std::string environment_value(const char *name, const char *fallback)
{
  const auto *value = std::getenv(name);
  return value == nullptr || *value == '\0' ? fallback : value;
}

Json::Value run_environment(const std::string &profile_id, const std::string &execution_mode)
{
  Json::Value environment(Json::objectValue);
  environment["simulator"]["name"] = "Gazebo Harmonic";
  environment["simulator"]["version"] = environment_value("GZ_VERSION", "unknown");
  environment["ros_distribution"] = environment_value("ROS_DISTRO", "unknown");
  environment["execution_profile"]["id"] = profile_id;
  environment["execution_profile"]["mode"] = execution_mode;
  environment["host"]["logical_cpu_count"] = std::thread::hardware_concurrency();

  struct utsname host_info {};
  if (uname(&host_info) == 0) {
    environment["host"]["os"] = host_info.sysname;
    environment["host"]["os_release"] = host_info.release;
  } else {
    environment["host"]["os"] = "unknown";
    environment["host"]["os_release"] = "unknown";
  }

  const long page_count = sysconf(_SC_PHYS_PAGES);
  const long page_size = sysconf(_SC_PAGESIZE);
  if (page_count > 0 && page_size > 0) {
    environment["host"]["memory_bytes"] = Json::UInt64(page_count) * Json::UInt64(page_size);
  }
  return environment;
}

}  // namespace

HealthyRunner::HealthyRunner(
  std::filesystem::path root,
  std::string batch_id,
  std::string run_id,
  Scenario scenario,
  std::string profile_id,
  std::string execution_mode)
: artifact_store_(root, batch_id, run_id),
  scenario_(std::move(scenario)),
  run_id_(std::move(run_id)),
  profile_id_(std::move(profile_id)),
  execution_mode_(std::move(execution_mode)),
  batch_id_(std::move(batch_id))
{
  if (run_id_.empty()) {
    throw std::invalid_argument("run_id must not be empty");
  }
  if (batch_id_.empty()) {
    throw std::invalid_argument("batch_id must not be empty");
  }
  if (profile_id_.empty()) {
    throw std::invalid_argument("profile_id must not be empty");
  }
}

void HealthyRunner::start_run()
{
  if (state_ != RunState::Created) {
    throw std::logic_error("HealthyRunner may only start once from Created state");
  }

  state_ = RunState::Starting;
  terminal_status_ = "starting";

  RunMetadata metadata;
  metadata.run_id = run_id_;
  metadata.batch_id = batch_id_;
  metadata.scenario_id = scenario_.scenario_id;
  metadata.configuration_hash = scenario_.scenario_id + ":" + profile_id_;
  metadata.profile_id = profile_id_;
  metadata.created_at = timestamp_seconds();
  metadata.created_at_clock_domain = "wall";
  metadata.clock_segment = 0;
  metadata.environment = run_environment(profile_id_, execution_mode_);
  artifact_store_.create_partial_run(metadata);
  append_state_event("run.lifecycle", state_name(state_));
}

void HealthyRunner::mark_ready()
{
  if (state_ != RunState::Starting && state_ != RunState::Ready) {
    throw std::logic_error("HealthyRunner is not yet ready to transition to Ready state");
  }

  state_ = RunState::Ready;
  append_state_event("readiness", state_name(state_));
}

void HealthyRunner::mark_running()
{
  if (state_ != RunState::Ready && state_ != RunState::Running) {
    throw std::logic_error("HealthyRunner must be ready before it can run");
  }

  state_ = RunState::Running;
  append_state_event("navigation", state_name(state_));
}

void HealthyRunner::finish_success(std::int64_t duration_ms)
{
  if (state_ != RunState::Running && state_ != RunState::Ready) {
    throw std::logic_error("HealthyRunner cannot succeed before it is running");
  }

  state_ = RunState::Succeeded;
  terminal_status_ = "succeeded";
  append_state_event("terminal", state_name(state_));
  artifact_store_.complete_run("succeeded", std::nullopt, duration_ms);
}

void HealthyRunner::finish_failure(const std::string &failure_class, std::int64_t duration_ms)
{
  if (state_ == RunState::Succeeded || state_ == RunState::Canceled || state_ == RunState::Interrupted) {
    throw std::logic_error("HealthyRunner cannot fail after it has terminalized");
  }

  state_ = RunState::Failed;
  terminal_status_ = "failed";
  append_state_event("terminal", state_name(state_));
  artifact_store_.complete_run("failed", failure_class, duration_ms);
}

void HealthyRunner::cancel(std::int64_t duration_ms)
{
  if (state_ == RunState::Succeeded || state_ == RunState::Canceled || state_ == RunState::Interrupted) {
    throw std::logic_error("HealthyRunner cannot cancel after it has terminalized");
  }

  state_ = RunState::Canceled;
  terminal_status_ = "canceled";
  append_state_event("terminal", state_name(state_));
  artifact_store_.complete_run("canceled", std::nullopt, duration_ms);
}

void HealthyRunner::interrupt(std::int64_t duration_ms)
{
  if (state_ == RunState::Succeeded || state_ == RunState::Canceled || state_ == RunState::Interrupted) {
    throw std::logic_error("HealthyRunner cannot interrupt after it has terminalized");
  }

  state_ = RunState::Interrupted;
  terminal_status_ = "interrupted";
  append_state_event("terminal", state_name(state_));
  artifact_store_.complete_run("interrupted", std::nullopt, duration_ms);
}

void HealthyRunner::append_state_event(const std::string &event_type, const std::string &state_name_value)
{
  RunEvent event;
  event.run_id = run_id_;
  event.event_id = run_id_ + "-event-" + std::to_string(++event_sequence_);
  event.sequence = event_sequence_;
  event.event_type = event_type;
  event.observed_at = timestamp_seconds();
  event.clock_domain = "wall";
  event.clock_segment = 0;
  event.source = "healthy_runner";
  event.payload["state"] = state_name_value;
  event.payload["scenario_id"] = scenario_.scenario_id;
  event.payload["profile_id"] = profile_id_;
  artifact_store_.append_event(event);
}

}  // namespace navigation_evaluation
