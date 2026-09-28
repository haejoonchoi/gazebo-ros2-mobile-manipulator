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

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

TEST(ReadinessCheck, MarksHealthySystemReady)
{
  navigation_evaluation::ReadinessRequirements requirements;
  requirements.clock_progress_timeout_s = 20.0;
  requirements.min_clock_progress_s = 0.5;
  requirements.tf_timeout_s = 5.0;
  requirements.localization_timeout_s = 5.0;
  requirements.sensor_freshness_timeout_s = 1.5;
  requirements.nav2_timeout_s = 10.0;

  navigation_evaluation::ReadinessObservation observation;
  observation.clock_progress_s = 1.0;
  observation.tf_connected = true;
  observation.localization_ready = true;
  observation.sensor_fresh = true;
  observation.nav2_ready = true;

  const auto result = navigation_evaluation::evaluate_readiness(requirements, observation);

  EXPECT_TRUE(result.ready);
  EXPECT_TRUE(result.failed_checks.empty());
  EXPECT_TRUE(result.failure_summary.empty());
}

TEST(ReadinessCheck, FlagsMissingChecks)
{
  navigation_evaluation::ReadinessRequirements requirements;
  requirements.clock_progress_timeout_s = 20.0;
  requirements.min_clock_progress_s = 0.5;
  requirements.tf_timeout_s = 5.0;
  requirements.localization_timeout_s = 5.0;
  requirements.sensor_freshness_timeout_s = 1.5;
  requirements.nav2_timeout_s = 10.0;

  navigation_evaluation::ReadinessObservation observation;
  observation.clock_progress_s = 0.0;
  observation.tf_connected = false;
  observation.localization_ready = false;
  observation.sensor_fresh = false;
  observation.nav2_ready = false;

  const auto result = navigation_evaluation::evaluate_readiness(requirements, observation);

  EXPECT_FALSE(result.ready);
  EXPECT_FALSE(result.failed_checks.empty());
  EXPECT_NE(result.failure_summary.find("clock"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("tf"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("localization"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("sensor"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("nav2"), std::string::npos);
}
