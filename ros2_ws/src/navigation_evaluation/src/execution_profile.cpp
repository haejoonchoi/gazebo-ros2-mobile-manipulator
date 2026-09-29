#include "navigation_evaluation/execution_profile.hpp"

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <regex>

namespace navigation_evaluation {
namespace {

template<typename T>
T required_value(const YAML::Node &node, const char *key, const std::filesystem::path &path)
{
  const auto value = node[key];
  if (!value) {
    throw ExecutionProfileError("Missing '" + std::string(key) + "' in " + path.string());
  }

  try {
    return value.as<T>();
  } catch (const YAML::Exception &error) {
    throw ExecutionProfileError(
      "Invalid '" + std::string(key) + "' in " + path.string() + ": " + error.what());
  }
}

double required_positive(const YAML::Node &node, const char *key, const std::filesystem::path &path)
{
  const double value = required_value<double>(node, key, path);
  if (!std::isfinite(value) || value <= 0.0) {
    throw ExecutionProfileError("'" + std::string(key) + "' must be finite and positive in " + path.string());
  }
  return value;
}

}  // namespace

ExecutionProfile load_execution_profile(const std::filesystem::path &path)
{
  if (path.extension() != ".yaml") {
    throw ExecutionProfileError("Execution profile must use the .yaml extension: " + path.string());
  }

  YAML::Node root;
  try {
    root = YAML::LoadFile(path.string());
  } catch (const YAML::Exception &error) {
    throw ExecutionProfileError("Unable to parse " + path.string() + ": " + error.what());
  }
  if (!root.IsMap()) {
    throw ExecutionProfileError("Execution profile must be a map: " + path.string());
  }

  ExecutionProfile profile;
  profile.profile_id = required_value<std::string>(root, "profile_id", path);
  if (!std::regex_match(profile.profile_id, std::regex("^[a-z0-9]+(-[a-z0-9]+)*$"))) {
    throw ExecutionProfileError("profile_id must use lowercase kebab-case in " + path.string());
  }
  profile.simulator = required_value<std::string>(root, "simulator", path);
  if (profile.simulator != "Gazebo Harmonic") {
    throw ExecutionProfileError("simulator must be Gazebo Harmonic in " + path.string());
  }
  profile.ros_distribution = required_value<std::string>(root, "ros_distribution", path);
  if (profile.ros_distribution != "jazzy") {
    throw ExecutionProfileError("ros_distribution must be jazzy in " + path.string());
  }

  const auto mode = required_value<std::string>(root, "mode", path);
  if (mode == "server_only") {
    profile.mode = ExecutionMode::ServerOnly;
  } else if (mode == "headless_rendering") {
    profile.mode = ExecutionMode::HeadlessRendering;
  } else if (mode == "graphical") {
    profile.mode = ExecutionMode::Graphical;
  } else {
    throw ExecutionProfileError("Unsupported execution mode: " + mode);
  }

  const auto resources = root["resource_profile"];
  if (!resources || !resources.IsMap()) {
    throw ExecutionProfileError("resource_profile must be a map in " + path.string());
  }
  profile.cpu_cores_min = required_value<int>(resources, "cpu_cores_min", path);
  profile.memory_gb_min = required_positive(resources, "memory_gb_min", path);
  profile.disk_free_gb_min = required_positive(resources, "disk_free_gb_min", path);
  profile.gpu_required = required_value<bool>(resources, "gpu_required", path);
  if (profile.cpu_cores_min < 4 || profile.memory_gb_min < 16.0 || profile.disk_free_gb_min < 5.0) {
    throw ExecutionProfileError("Resource minimums must be at least 4 CPU cores, 16 GB RAM, and 5 GB disk");
  }
  if (profile.gpu_required) {
    throw ExecutionProfileError("The constrained-hardware profile must not require a GPU");
  }

  const auto readiness = root["readiness"];
  if (!readiness || !readiness.IsMap()) {
    throw ExecutionProfileError("readiness must be a map in " + path.string());
  }
  profile.clock_progress_timeout_s = required_positive(readiness, "clock_progress_timeout_s", path);
  profile.startup_timeout_s = required_positive(readiness, "startup_timeout_s", path);
  if (profile.clock_progress_timeout_s > 20.0 || profile.startup_timeout_s > 120.0) {
    throw ExecutionProfileError("Readiness limits may not exceed 20 seconds for clock or 120 seconds for startup");
  }
  required_value<bool>(readiness, "localization_required", path);
  required_value<bool>(readiness, "nav2_required", path);
  required_value<bool>(readiness, "tf_required", path);

  const auto world = root["world"];
  const auto robot = root["robot"];
  const auto bridge = root["bridge"];
  const auto limits = root["known_limits"];
  if (!world || !world.IsMap() || !robot || !robot.IsMap() || !bridge || !bridge.IsMap() ||
    !limits || !limits.IsMap())
  {
    throw ExecutionProfileError("world, robot, bridge, and known_limits maps are required in " + path.string());
  }
  required_value<std::string>(world, "name", path);
  required_value<std::string>(world, "file", path);
  required_value<std::string>(world, "physics", path);
  required_value<std::string>(robot, "name", path);
  required_value<std::string>(robot, "urdf", path);
  const auto sensor_frames = robot["sensor_frames"];
  if (!sensor_frames || !sensor_frames.IsSequence() || sensor_frames.size() < 3) {
    throw ExecutionProfileError("robot.sensor_frames must contain the base and required sensor frames");
  }
  for (const auto *key : {"clock_topic", "odom_topic", "scan_topic", "imu_topic", "tf_topic"}) {
    const auto topic = required_value<std::string>(bridge, key, path);
    if (topic.empty() || topic.front() != '/') {
      throw ExecutionProfileError("bridge topics must be absolute ROS topic names in " + path.string());
    }
  }
  required_value<bool>(limits, "realtime_factor_not_required", path);
  required_value<bool>(limits, "visual_rendering_optional", path);
  required_value<bool>(limits, "headless_mode_available_as_override", path);

  return profile;
}

}  // namespace navigation_evaluation