#include "navigation_evaluation/execution_profile.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path write_profile(const std::string &contents)
{
  const auto path = std::filesystem::temp_directory_path() / "navigation_execution_profile_test.yaml";
  std::ofstream output(path);
  output << contents;
  output.close();
  return path;
}

const std::string kInvalidProfile = R"(
profile_id: headless-low-resource
simulator: Gazebo Harmonic
ros_distribution: jazzy
mode: server_only
resource_profile:
  cpu_cores_min: 2
  memory_gb_min: 16
  disk_free_gb_min: 5
  gpu_required: false
readiness:
  clock_progress_timeout_s: 25
  startup_timeout_s: 120
  localization_required: true
  nav2_required: true
  tf_required: true
world: {name: low_resource_world, file: worlds/low_resource_world.sdf, physics: ode}
robot:
  name: mobile_manipulator
  urdf: urdf/mobile_manipulator.urdf
  sensor_frames: [base_link, lidar_link, imu_link]
bridge:
  clock_topic: /clock
  odom_topic: /odom
  scan_topic: /scan_raw
  imu_topic: /imu/data
  tf_topic: /tf
known_limits:
  realtime_factor_not_required: true
  visual_rendering_optional: true
  headless_mode_available_as_override: true
)";

}  // namespace

TEST(ExecutionProfile, LoadsSupportedHeadlessProfile)
{
  const auto profile = navigation_evaluation::load_execution_profile(
    NAVIGATION_EVALUATION_EXECUTION_PROFILE_PATH);

  EXPECT_EQ(profile.profile_id, "headless-low-resource");
  EXPECT_EQ(profile.simulator, "Gazebo Harmonic");
  EXPECT_EQ(profile.ros_distribution, "jazzy");
  EXPECT_EQ(profile.mode, navigation_evaluation::ExecutionMode::ServerOnly);
  EXPECT_EQ(profile.cpu_cores_min, 4);
  EXPECT_DOUBLE_EQ(profile.memory_gb_min, 16.0);
  EXPECT_DOUBLE_EQ(profile.clock_progress_timeout_s, 20.0);
  EXPECT_DOUBLE_EQ(profile.startup_timeout_s, 120.0);
}

TEST(ExecutionProfile, RejectsProfileOutsideResourceAndReadinessLimits)
{
  const auto path = write_profile(kInvalidProfile);
  EXPECT_THROW(
    navigation_evaluation::load_execution_profile(path),
    navigation_evaluation::ExecutionProfileError);
  std::filesystem::remove(path);
}
