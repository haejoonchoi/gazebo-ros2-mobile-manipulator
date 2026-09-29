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

#include <string>
#include <vector>

namespace navigation_evaluation {

struct ReadinessRequirements
{
  double clock_progress_timeout_s{20.0};
  double min_clock_progress_s{0.5};
  double tf_timeout_s{5.0};
  double localization_timeout_s{5.0};
  double sensor_freshness_timeout_s{1.5};
  double nav2_timeout_s{10.0};
};

struct ReadinessObservation
{
  double clock_progress_s{0.0};
  double clock_elapsed_s{0.0};
  double tf_elapsed_s{0.0};
  double localization_elapsed_s{0.0};
  double sensor_elapsed_s{0.0};
  double nav2_elapsed_s{0.0};
  unsigned int clock_publisher_count{0};
  bool tf_connected{false};
  bool localization_ready{false};
  bool raw_lidar_fresh{false};
  bool impaired_lidar_fresh{false};
  bool imu_fresh{false};
  std::string imu_frame_id;
  bool nav2_ready{false};
  bool reset_services_ready{false};
  bool execution_profile_valid{false};
  bool resource_profile_supported{false};
};

struct ReadinessResult
{
  bool ready{false};
  bool timed_out{false};
  std::vector<std::string> failed_checks;
  std::string failure_summary;
  std::string failure_class;
};

ReadinessResult evaluate_readiness(
  const ReadinessRequirements &requirements,
  const ReadinessObservation &observation);

}  // namespace navigation_evaluation
