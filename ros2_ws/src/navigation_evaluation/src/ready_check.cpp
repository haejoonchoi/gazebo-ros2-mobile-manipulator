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
#include "navigation_evaluation/ready_check.hpp"

#include <algorithm>
#include <sstream>

namespace navigation_evaluation {
namespace {

void append_failure(std::vector<std::string> &failed_checks, const std::string &name)
{
  if (std::find(failed_checks.begin(), failed_checks.end(), name) == failed_checks.end()) {
    failed_checks.push_back(name);
  }
}

}  // namespace

ReadinessResult evaluate_readiness(
  const ReadinessRequirements &requirements,
  const ReadinessObservation &observation)
{
  ReadinessResult result;
  std::vector<std::string> failed_checks;
  const auto add_timeout = [&](bool failed, double elapsed_s, double timeout_s, const char *name) {
      if (failed && elapsed_s >= timeout_s) {
        append_failure(failed_checks, std::string(name) + "_timeout");
        result.timed_out = true;
      }
    };

  if (observation.clock_publisher_count != 1) {
    append_failure(failed_checks, "clock_publisher_count");
  }
  if (observation.clock_progress_s < requirements.min_clock_progress_s) {
    append_failure(failed_checks, "clock");
  }
  add_timeout(
    observation.clock_publisher_count != 1 ||
    observation.clock_progress_s < requirements.min_clock_progress_s,
    observation.clock_elapsed_s, requirements.clock_progress_timeout_s, "clock");
  if (!observation.tf_connected) {
    append_failure(failed_checks, "tf");
  }
  add_timeout(!observation.tf_connected, observation.tf_elapsed_s, requirements.tf_timeout_s, "tf");
  if (!observation.localization_ready) {
    append_failure(failed_checks, "localization");
  }
  add_timeout(
    !observation.localization_ready, observation.localization_elapsed_s,
    requirements.localization_timeout_s, "localization");

  const bool sensor_ready = observation.raw_lidar_fresh && observation.impaired_lidar_fresh &&
    observation.imu_fresh && observation.imu_frame_id == "imu_link";
  if (!sensor_ready) {
    append_failure(failed_checks, "sensor");
  }
  if (!observation.raw_lidar_fresh) {
    append_failure(failed_checks, "raw_lidar");
  }
  if (!observation.impaired_lidar_fresh) {
    append_failure(failed_checks, "impaired_lidar");
  }
  if (!observation.imu_fresh) {
    append_failure(failed_checks, "imu_freshness");
  }
  if (observation.imu_frame_id != "imu_link") {
    append_failure(failed_checks, "imu_frame");
  }
  add_timeout(!sensor_ready, observation.sensor_elapsed_s, requirements.sensor_freshness_timeout_s, "sensor");
  if (!observation.nav2_ready) {
    append_failure(failed_checks, "nav2");
  }
  add_timeout(!observation.nav2_ready, observation.nav2_elapsed_s, requirements.nav2_timeout_s, "nav2");
  if (!observation.reset_services_ready) {
    append_failure(failed_checks, "reset_services");
  }
  if (!observation.execution_profile_valid) {
    append_failure(failed_checks, "execution_profile");
  }
  if (!observation.resource_profile_supported) {
    append_failure(failed_checks, "resource_profile");
  }

  result.failed_checks = failed_checks;
  if (failed_checks.empty()) {
    result.ready = true;
    result.failure_summary.clear();
    result.failure_class.clear();
    return result;
  }

  result.failure_class = result.timed_out ? "readiness_timeout" : "readiness_not_ready";
  std::ostringstream stream;
  stream << "not ready: ";
  for (std::size_t index = 0; index < failed_checks.size(); ++index) {
    if (index > 0) {
      stream << ", ";
    }
    stream << failed_checks[index];
  }
  result.failure_summary = stream.str();
  return result;
}

}  // namespace navigation_evaluation
