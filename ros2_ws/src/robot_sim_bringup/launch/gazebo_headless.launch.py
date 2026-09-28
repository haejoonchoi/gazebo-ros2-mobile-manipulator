#!/usr/bin/env python3
# Copyright 2026 Open Source Robotics Foundation, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Launch the Gazebo world in headless server mode."""

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    LogInfo,
    OpaqueFunction,
)
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node


def launch_gazebo(context, *args, **kwargs):
    """Assemble the headless Gazebo world, robot publisher, and bridge."""
    package_share = get_package_share_directory('robot_sim_bringup')
    world_file = os.path.join(
        package_share,
        'worlds',
        'low_resource_world.sdf',
    )
    bridge_config = os.path.join(
        package_share,
        'config',
        'ros_gz_bridge.yaml',
    )
    urdf_file = os.path.join(
        package_share,
        'urdf',
        'mobile_manipulator.urdf',
    )

    use_sim_time = LaunchConfiguration(
        'use_sim_time',
        default='true',
    )
    world_name = LaunchConfiguration(
        'world_name',
        default='low_resource_world',
    )
    headless = LaunchConfiguration('headless', default='true')

    gazebo_cmd = ['gz', 'sim', '-v', '4', '-r', world_file]
    if headless.perform(context) == 'true':
        gazebo_cmd.insert(4, '-s')

    log_message = (
        'Launching Gazebo world: '
        + world_name.perform(context)
        + ' (headless='
        + headless.perform(context)
        + ')'
    )

    return [
        LogInfo(msg=log_message),
        ExecuteProcess(
            cmd=gazebo_cmd,
            output='screen',
            additional_env={'GZ_SIM_RESOURCE_PATH': package_share},
        ),
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            arguments=[urdf_file],
            parameters=[{'use_sim_time': use_sim_time.perform(context)}],
            output='screen',
        ),
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            name='ros_gz_bridge',
            arguments=[
                '--ros-args',
                '--params-file',
                bridge_config,
            ],
            parameters=[{'use_sim_time': use_sim_time.perform(context)}],
            output='screen',
        ),
    ]


def generate_launch_description():
    """Return the headless Gazebo launch description."""
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                'world_name',
                default_value='low_resource_world',
            ),
            DeclareLaunchArgument('use_sim_time', default_value='true'),
            DeclareLaunchArgument('headless', default_value='true'),
            OpaqueFunction(function=launch_gazebo),
        ]
    )
