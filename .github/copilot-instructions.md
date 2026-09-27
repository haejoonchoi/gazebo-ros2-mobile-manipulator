# Copilot workspace instructions

- Before running any terminal command, explain what the command does, why it is needed, and what result to expect.
- Keep commands focused on the current task and avoid unnecessary broad operations.
- When working in the ROS2 workspace, source the appropriate environment first and prefer the existing project build helpers when available.
- If a command fails, explain the likely cause and the next diagnostic step before retrying.
- Teach the ROS2 workflow while helping in this project: source the ROS environment, inspect the package layout, modify the relevant package, build it, then validate the behavior with a launch or test command.
- For this repository, the normal ROS2 development flow is:
  1. source /opt/ros/jazzy/setup.bash
  2. source ros2_ws/install/setup.bash when working with built packages
  3. inspect the package under ros2_ws/src/<package_name> and its package.xml / CMakeLists.txt
  4. make the smallest change that addresses the issue or feature
  5. build the affected package or workspace with colcon
  6. run the relevant launch file, node, or test to validate behavior
- Prefer package-scoped builds for learning and debugging, for example: colcon build --packages-select <package_name> --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -G Ninja
- When a package has launch files, config, URDF/Xacro, or world assets, check those files before changing code so the user understands how the system is assembled in ROS.
- Explain the purpose of common ROS2 files: package.xml declares dependencies and metadata; CMakeLists.txt defines build targets; launch files start the graph; config files hold parameters; URDF/Xacro describe robot geometry and joints.
- When the user is learning the project, prefer concise explanations of the system architecture, package boundaries, and execution flow instead of only patching code.
- If a dependency or import issue appears, explain that ROS2 typically needs the environment to be sourced and that packages must be built in the correct workspace before they can be discovered by ros2 run / ros2 launch.
- Read project docs under specs/ before implementation when the task is feature work, so the instructions can connect the code to the intended behavior and architecture.
- Use the repo’s existing tasks such as ROS2: source workspace and ROS2: build workspace when they match the workflow, and explain why they are being used.
