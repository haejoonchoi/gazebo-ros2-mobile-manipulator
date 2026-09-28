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
"""Smoke tests for the Gazebo launch defaults."""

import os
import subprocess
import unittest


class GazeboLaunchDefaultsTest(unittest.TestCase):
    """Verify the default Gazebo launch mode matches the intended profile."""

    def _show_args(self, launch_file):
        result = subprocess.run(
            ['ros2', 'launch', 'robot_sim_bringup', launch_file, '--show-args'],
            capture_output=True,
            text=True,
            env=os.environ.copy(),
            check=False,
        )
        self.assertEqual(
            result.returncode,
            0,
            f'ros2 launch {launch_file} --show-args failed:\n'
            f'{result.stdout}\n{result.stderr}',
        )
        return result.stdout + result.stderr

    def test_default_gui_launch_uses_graphical_mode(self):
        output = self._show_args('gazebo.launch.py')
        self.assertIn("'headless':", output)
        self.assertIn("(default: 'false')", output)

    def test_headless_launch_uses_server_mode_by_default(self):
        output = self._show_args('gazebo_headless.launch.py')
        self.assertIn("'headless':", output)
        self.assertIn("(default: 'true')", output)


if __name__ == '__main__':
    unittest.main()
