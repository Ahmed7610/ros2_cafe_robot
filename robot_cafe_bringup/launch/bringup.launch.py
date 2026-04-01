import os
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    IncludeLaunchDescription,
)
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    # Package paths
    bringup_pkg = FindPackageShare("robot_cafe_bringup").find("robot_cafe_bringup")
    manager_pkg = FindPackageShare("robot_manager").find("robot_manager")
    interface_pkg = FindPackageShare("robot_manager_interface").find(
        "robot_manager_interface"
    )
    sim_pkg = FindPackageShare("robot_cafe_sim").find("robot_cafe_sim")

    # Default launch files
    default_manager_launch = os.path.join(manager_pkg, "launch", "manager.launch.py")
    default_interface_launch = os.path.join(
        interface_pkg, "launch", "interface.launch.py"
    )
    navigation_script = os.path.join(sim_pkg, "scripts", "run_navigation.sh")

    # ── arguments ────────────────────────────────────────────────────────────

    args = [
        DeclareLaunchArgument(
            "manager_launch_file", default_value=default_manager_launch
        ),
        DeclareLaunchArgument(
            "interface_launch_file", default_value=default_interface_launch
        ),
        DeclareLaunchArgument("slam_mode", default_value="false"),
        DeclareLaunchArgument("use_manager", default_value="true"),
        DeclareLaunchArgument("use_interface", default_value="true"),
    ]

    # ── processes / includes ────────────────────────────────────────────────

    navigation_normal = ExecuteProcess(
        cmd=["bash", navigation_script],
        output="screen",
        condition=IfCondition(
            PythonExpression(["'", LaunchConfiguration("slam_mode"), "' == 'false'"])
        ),
    )

    navigation_slam = ExecuteProcess(
        cmd=["bash", navigation_script, "slam"],
        output="screen",
        condition=IfCondition(
            PythonExpression(["'", LaunchConfiguration("slam_mode"), "' == 'true'"])
        ),
    )

    manager = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([LaunchConfiguration("manager_launch_file")]),
        condition=IfCondition(LaunchConfiguration("use_manager")),
    )

    interface = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([LaunchConfiguration("interface_launch_file")]),
        condition=IfCondition(LaunchConfiguration("use_interface")),
    )

    # ── launch description ───────────────────────────────────────────────────

    ld = LaunchDescription(args)
    ld.add_action(navigation_normal)
    ld.add_action(navigation_slam)
    ld.add_action(manager)
    ld.add_action(interface)

    return ld
