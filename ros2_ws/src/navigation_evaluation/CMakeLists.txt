cmake_minimum_required(VERSION 3.16)
project(navigation_evaluation)

find_package(ament_cmake REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(yaml-cpp REQUIRED)

set(AMENT_LINT_AUTO_FILE_EXCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_evaluation/scenario.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/navigation_evaluation/artifact_store.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/scenario.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/artifact_store.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/test_scenario_schema.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/test_artifact_store.cpp"
)
set(AMENT_LINT_AUTO_EXCLUDE
  ament_cmake_cpplint
  ament_cmake_uncrustify
)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  ament_add_gtest(test_scenario_schema test/test_scenario_schema.cpp)
  target_include_directories(test_scenario_schema PRIVATE include)
  target_link_libraries(test_scenario_schema navigation_evaluation_core)
  target_compile_definitions(
    test_scenario_schema
    PRIVATE NAVIGATION_EVALUATION_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/fixtures")

  ament_add_gtest(test_execution_profile test/test_execution_profile.cpp)
  target_include_directories(test_execution_profile PRIVATE include)
  target_link_libraries(test_execution_profile navigation_evaluation_core)
  target_compile_definitions(
    test_execution_profile
    PRIVATE NAVIGATION_EVALUATION_EXECUTION_PROFILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../robot_sim_bringup/config/gazebo_execution_profile.yaml")

  ament_add_gtest(test_artifact_store test/test_artifact_store.cpp)
  target_include_directories(test_artifact_store PRIVATE include)
  target_link_libraries(test_artifact_store navigation_evaluation_core)

  ament_add_gtest(test_ready_check test/test_ready_check.cpp)
  target_include_directories(test_ready_check PRIVATE include)
  target_link_libraries(test_ready_check navigation_evaluation_core)

  ament_add_gtest(test_healthy_runner test/test_healthy_runner.cpp)
  target_include_directories(test_healthy_runner PRIVATE include)
  target_link_libraries(test_healthy_runner navigation_evaluation_core)
endif()

add_library(navigation_evaluation_core
  src/execution_profile.cpp
  src/scenario.cpp
  src/artifact_store.cpp
  src/ready_check.cpp
  src/healthy_runner.cpp
)
target_include_directories(navigation_evaluation_core PUBLIC include)
target_link_libraries(navigation_evaluation_core yaml-cpp JsonCpp::JsonCpp)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY config/
  DESTINATION share/${PROJECT_NAME}
)

install(
  TARGETS navigation_evaluation_core
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
