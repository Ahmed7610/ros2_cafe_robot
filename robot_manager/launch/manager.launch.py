from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    # ── arguments ────────────────────────────────────────────────────────────

    args = [
        DeclareLaunchArgument("use_sim_time", default_value="true"),
    ]

    # ── nodes ────────────────────────────────────────────────────────────────

    cafe_task_manager = Node(
        package="robot_manager",
        executable="cafe_task_manager",
        name="cafe_task_manager",
        output="screen",
        parameters=[{"use_sim_time": LaunchConfiguration("use_sim_time")}],
    )

    # ── launch description ───────────────────────────────────────────────────

    ld = LaunchDescription(args)
    ld.add_action(cafe_task_manager)
    return ld
