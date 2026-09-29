#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>

namespace navigation_evaluation {

enum class ExecutionMode
{
  ServerOnly,
  HeadlessRendering,
  Graphical
};

struct ExecutionProfile
{
  std::string profile_id;
  std::string simulator;
  std::string ros_distribution;
  ExecutionMode mode{ExecutionMode::ServerOnly};
  int cpu_cores_min{0};
  double memory_gb_min{0.0};
  double disk_free_gb_min{0.0};
  bool gpu_required{false};
  double clock_progress_timeout_s{0.0};
  double startup_timeout_s{0.0};
};

class ExecutionProfileError : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

ExecutionProfile load_execution_profile(const std::filesystem::path &path);

}  // namespace navigation_evaluation