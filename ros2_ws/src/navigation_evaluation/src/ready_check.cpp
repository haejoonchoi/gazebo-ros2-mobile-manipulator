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

  if (observation.clock_progress_s < requirements.min_clock_progress_s) {
    append_failure(failed_checks, "clock");
  }
  if (!observation.tf_connected) {
    append_failure(failed_checks, "tf");
  }
  if (!observation.localization_ready) {
    append_failure(failed_checks, "localization");
  }
  if (!observation.sensor_fresh) {
    append_failure(failed_checks, "sensor");
  }
  if (!observation.nav2_ready) {
    append_failure(failed_checks, "nav2");
  }

  result.failed_checks = failed_checks;
  if (failed_checks.empty()) {
    result.ready = true;
    result.failure_summary = "ready";
    return result;
  }

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
