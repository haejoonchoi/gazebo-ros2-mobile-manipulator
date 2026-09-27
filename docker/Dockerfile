FROM ros:jazzy-ros-base

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions \
    python3-pip \
    ros-jazzy-navigation2 \
    ros-jazzy-nav2-bringup \
    ros-jazzy-tf2-tools \
    ros-jazzy-rviz2 \
    libjsoncpp-dev \
    qt6-base-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces/gazebo-ros2-mobile-manipulator

COPY ros2_ws/src ros2_ws/src

RUN source /opt/ros/jazzy/setup.bash \
    && cd ros2_ws \
    && colcon build --symlink-install

CMD ["bash"]
