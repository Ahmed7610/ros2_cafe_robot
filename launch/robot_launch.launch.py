import os
from launch import LaunchDescription
from launch.actions import AppendEnvironmentVariable, DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    pkg    = FindPackageShare("ros2_cafe_robot").find("ros2_cafe_robot")
    pkg_gz = FindPackageShare("ros_gz_sim").find("ros_gz_sim")

    world_path   = PathJoinSubstitution([pkg, "worlds", LaunchConfiguration("world_file")])
    default_rviz = PathJoinSubstitution([pkg, "rviz", "rviz_parameters.rviz"])

    # ── arguments ────────────────────────────────────────────────────────────

    args = [
        DeclareLaunchArgument("enable_odom_tf",      default_value="false", choices=["true", "false"]),
        DeclareLaunchArgument("headless",             default_value="False"),
        DeclareLaunchArgument("robot_name",           default_value="ros2_cafe_robot"),
        DeclareLaunchArgument("rviz_config_file",     default_value=default_rviz),
        DeclareLaunchArgument("load_controllers",     default_value="true"),
        DeclareLaunchArgument("jsp_gui",              default_value="false"),
        DeclareLaunchArgument("use_rviz",             default_value="true"),
        DeclareLaunchArgument("use_gazebo",           default_value="true"),
        DeclareLaunchArgument("use_robot_state_pub",  default_value="true"),
        DeclareLaunchArgument("use_sim_time",         default_value="true"),
        DeclareLaunchArgument("world_file",           default_value="cafe.world"),
        DeclareLaunchArgument("x",                    default_value="-0.2786"),
        DeclareLaunchArgument("y",                    default_value="0.0"),
        DeclareLaunchArgument("z",                    default_value="0.1898"),
        DeclareLaunchArgument("roll",                 default_value="0.0"),
        DeclareLaunchArgument("pitch",                default_value="0.001"),
        DeclareLaunchArgument("yaw",                  default_value="0.0"),
    ]

    # ── includes / nodes ─────────────────────────────────────────────────────

    set_gz_resource_path = AppendEnvironmentVariable(
        "GZ_SIM_RESOURCE_PATH", os.path.dirname(pkg)
    )

    robot_state_publisher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(pkg, "launch", "robot_state_publisher.launch.py")),
        launch_arguments={
            "enable_odom_tf":   LaunchConfiguration("enable_odom_tf"),
            "jsp_gui":          LaunchConfiguration("jsp_gui"),
            "rviz_config_file": LaunchConfiguration("rviz_config_file"),
            "use_rviz":         LaunchConfiguration("use_rviz"),
            "use_gazebo":       LaunchConfiguration("use_gazebo"),
            "use_sim_time":     LaunchConfiguration("use_sim_time"),
        }.items(),
        condition=IfCondition(LaunchConfiguration("use_robot_state_pub")),
    )

    controllers = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(pkg, "launch", "controllers.launch.py")),
        launch_arguments={"use_sim_time": LaunchConfiguration("use_sim_time")}.items(),
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(pkg_gz, "launch", "gz_sim.launch.py")),
        launch_arguments={"gz_args": ["-r ", world_path]}.items(),
    )

    gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        parameters=[{"config_file": os.path.join(pkg, "config/ros_gz_bridge.yaml")}],
        output="screen",
    )

    image_bridge = Node(
        package="ros_gz_image",
        executable="image_bridge",
        arguments=["/cam_1/image"],
        remappings=[("/cam_1/image", "/cam_1/color/image_raw")],
    )

    spawn_robot = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-topic",          "/robot_description",
            "-name",           LaunchConfiguration("robot_name"),
            "-allow_renaming", "true",
            "-x",              LaunchConfiguration("x"),
            "-y",              LaunchConfiguration("y"),
            "-z",              LaunchConfiguration("z"),
            "-R",              LaunchConfiguration("roll"),
            "-P",              LaunchConfiguration("pitch"),
            "-Y",              LaunchConfiguration("yaw"),
        ],
    )

    # ── launch description ───────────────────────────────────────────────────

    ld = LaunchDescription(args)
    ld.add_action(set_gz_resource_path)
    ld.add_action(robot_state_publisher)
    ld.add_action(controllers)
    ld.add_action(gazebo)
    ld.add_action(gz_bridge)
    ld.add_action(image_bridge)
    ld.add_action(spawn_robot)
    return ld
