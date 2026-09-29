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

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

TEST(HealthyRunner, RunsFromReadyToTerminalArtifact)
{
  const auto root = std::filesystem::temp_directory_path() / "navigation_healthy_runner";
  std::filesystem::remove_all(root);

  navigation_evaluation::Scenario scenario =
    navigation_evaluation::load_scenario(std::filesystem::path(
      "ros2_ws/src/navigation_evaluation/test/fixtures/healthy.yaml"));

  navigation_evaluation::HealthyRunner runner(
    root, "batch-1", "run-healthy", scenario, "headless-low-resource");

  EXPECT_EQ(runner.state(), navigation_evaluation::RunState::Created);

  runner.start_run();
  EXPECT_EQ(runner.state(), navigation_evaluation::RunState::Starting);

  runner.mark_ready();
  EXPECT_EQ(runner.state(), navigation_evaluation::RunState::Ready);

  runner.mark_running();
  EXPECT_EQ(runner.state(), navigation_evaluation::RunState::Running);

  runner.finish_success(1250);

  EXPECT_EQ(runner.state(), navigation_evaluation::RunState::Succeeded);
  EXPECT_EQ(runner.terminal_status(), "succeeded");
  EXPECT_EQ(runner.run_path().filename(), "run.json");

  std::ifstream input(runner.run_path());
  Json::Value record;
  input >> record;
  EXPECT_EQ(record["scenario_id"].asString(), "healthy");
  EXPECT_EQ(record["profile_id"].asString(), "headless-low-resource");
  EXPECT_EQ(record["schema_version"].asInt(), 1);
  EXPECT_EQ(record["environment"]["simulator"]["name"].asString(), "Gazebo Harmonic");
  EXPECT_EQ(record["environment"]["execution_profile"]["id"].asString(), "headless-low-resource");
  EXPECT_EQ(record["environment"]["execution_profile"]["mode"].asString(), "server_only");
  EXPECT_GT(record["environment"]["host"]["logical_cpu_count"].asUInt(), 0U);
  EXPECT_FALSE(record["environment"]["host"]["os"].asString().empty());
  EXPECT_FALSE(record["environment"]["ros_distribution"].asString().empty());
  EXPECT_EQ(record["terminal_status"].asString(), "succeeded");
  EXPECT_EQ(record["duration_ms"].asInt64(), 1250);

  std::filesystem::remove_all(root);
}
