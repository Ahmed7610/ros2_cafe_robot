from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    args = [
        DeclareLaunchArgument("use_sim_time", default_value="true"),
    ]

    web_bridge = ExecuteProcess(
        cmd=["ros2", "run", "robot_manager_interface", "web_bridge"],
        output="screen",
    )

    ld = LaunchDescription(args)
    ld.add_action(web_bridge)

    return ld
