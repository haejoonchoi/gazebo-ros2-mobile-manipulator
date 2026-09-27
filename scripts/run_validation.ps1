param(
    [string]$Workspace = "ros2_ws"
)

$ErrorActionPreference = "Stop"

if (Get-Command colcon -ErrorAction SilentlyContinue) {
    Push-Location $Workspace
    try {
        colcon build --symlink-install
        colcon test
        colcon test-result --verbose
    }
    finally {
        Pop-Location
    }
}
else {
    throw "colcon is required for ROS 2 validation but was not found on PATH."
}
