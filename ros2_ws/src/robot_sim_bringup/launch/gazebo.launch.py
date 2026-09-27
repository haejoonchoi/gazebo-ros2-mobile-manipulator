#!/usr/bin/env python3

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, LogInfo, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_gazebo(context, *args, **kwargs):
    package_share = get_package_share_directory("robot_sim_bringup")
    world_file = os.path.join(package_share, "worlds", "low_resource_world.sdf")
    bridge_config = os.path.join(package_share, "config", "ros_gz_bridge.yaml")
    urdf_file = os.path.join(package_share, "urdf", "mobile_manipulator.urdf")

    use_sim_time = LaunchConfiguration("use_sim_time", default="true")
    world_name = LaunchConfiguration("world_name", default="low_resource_world")
    headless = LaunchConfiguration("headless", default="false")

    gazebo_cmd = ["gz", "sim", "-v", "4", "-r", world_file]
    if headless.perform(context) == "true":
        gazebo_cmd.insert(4, "-s")

    return [
        LogInfo(msg=["Launching Gazebo world: ", world_name.perform(context), " (headless=", headless.perform(context), ")"]),
        ExecuteProcess(
            cmd=gazebo_cmd,
            output="screen",
            additional_env={"GZ_SIM_RESOURCE_PATH": package_share},
        ),
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="robot_state_publisher",
            arguments=[urdf_file],
            parameters=[{"use_sim_time": use_sim_time.perform(context)}],
            output="screen",
        ),
        Node(
            package="ros_gz_bridge",
            executable="parameter_bridge",
            name="ros_gz_bridge",
            arguments=["--ros-args", "--params-file", bridge_config],
            parameters=[{"use_sim_time": use_sim_time.perform(context)}],
            output="screen",
        ),
    ]


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument("world_name", default_value="low_resource_world"),
            DeclareLaunchArgument("use_sim_time", default_value="true"),
            DeclareLaunchArgument("headless", default_value="false"),
            OpaqueFunction(function=launch_gazebo),
        ]
    )
