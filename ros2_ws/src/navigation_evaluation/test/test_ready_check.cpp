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
  observation.clock_publisher_count = 1;
  observation.tf_connected = true;
  observation.localization_ready = true;
  observation.raw_lidar_fresh = true;
  observation.impaired_lidar_fresh = true;
  observation.imu_fresh = true;
  observation.imu_frame_id = "imu_link";
  observation.nav2_ready = true;
  observation.reset_services_ready = true;
  observation.execution_profile_valid = true;
  observation.resource_profile_supported = true;

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
  observation.clock_publisher_count = 0;
  observation.tf_connected = false;
  observation.localization_ready = false;
  observation.raw_lidar_fresh = false;
  observation.impaired_lidar_fresh = false;
  observation.imu_fresh = false;
  observation.imu_frame_id = "";
  observation.nav2_ready = false;
  observation.reset_services_ready = false;
  observation.execution_profile_valid = false;
  observation.resource_profile_supported = false;

  const auto result = navigation_evaluation::evaluate_readiness(requirements, observation);

  EXPECT_FALSE(result.ready);
  EXPECT_FALSE(result.failed_checks.empty());
  EXPECT_NE(result.failure_summary.find("clock"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("tf"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("localization"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("sensor"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("nav2"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("clock_publisher_count"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("raw_lidar"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("impaired_lidar"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("imu_frame"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("reset_services"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("execution_profile"), std::string::npos);
  EXPECT_NE(result.failure_summary.find("resource_profile"), std::string::npos);
}

TEST(ReadinessCheck, ClassifiesExpiredSubsystemDeadline)
{
  navigation_evaluation::ReadinessRequirements requirements;
  navigation_evaluation::ReadinessObservation observation;
  observation.clock_publisher_count = 1;
  observation.tf_connected = true;
  observation.localization_ready = true;
  observation.raw_lidar_fresh = true;
  observation.impaired_lidar_fresh = true;
  observation.imu_fresh = true;
  observation.imu_frame_id = "imu_link";
  observation.nav2_elapsed_s = requirements.nav2_timeout_s;
  observation.reset_services_ready = true;
  observation.execution_profile_valid = true;
  observation.resource_profile_supported = true;

  const auto result = navigation_evaluation::evaluate_readiness(requirements, observation);

  EXPECT_FALSE(result.ready);
  EXPECT_TRUE(result.timed_out);
  EXPECT_EQ(result.failure_class, "readiness_timeout");
  EXPECT_NE(result.failure_summary.find("nav2_timeout"), std::string::npos);
}
