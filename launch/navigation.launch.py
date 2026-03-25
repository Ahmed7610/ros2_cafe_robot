import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    pkg             = FindPackageShare("ros2_cafe_robot").find("ros2_cafe_robot")
    nav2_launch_dir = os.path.join(FindPackageShare("nav2_bringup").find("nav2_bringup"), "launch")

    default_gazebo_launch = os.path.join(pkg, "launch/robot_launch.launch.py")
    default_ekf_launch    = os.path.join(pkg, "launch/ekf_launch.launch.py")
    default_ekf_config    = os.path.join(pkg, "config/ekf.yaml")
    default_rviz          = os.path.join(pkg, "rviz/nav2_default_view.rviz")
    default_nav2_params   = os.path.join(pkg, "config/navigation_parameters.yaml")
    default_map           = os.path.join(pkg, "maps/cafe_world_map.yaml")

    # ── arguments ────────────────────────────────────────────────────────────

    args = [
        DeclareLaunchArgument("autostart",           default_value="true"),
        DeclareLaunchArgument("enable_odom_tf",      default_value="false", choices=["true", "false"]),
        DeclareLaunchArgument("ekf_config_file",     default_value=default_ekf_config),
        DeclareLaunchArgument("ekf_launch_file",     default_value=default_ekf_launch),
        DeclareLaunchArgument("gazebo_launch_file",  default_value=default_gazebo_launch),
        DeclareLaunchArgument("map",                 default_value=default_map),
        DeclareLaunchArgument("namespace",           default_value=""),
        DeclareLaunchArgument("nav2_params_file",    default_value=default_nav2_params),
        DeclareLaunchArgument("rviz_config_file",    default_value=default_rviz),
        DeclareLaunchArgument("slam",                default_value="False"),
        DeclareLaunchArgument("use_composition",     default_value="True"),
        DeclareLaunchArgument("use_namespace",       default_value="false"),
        DeclareLaunchArgument("use_respawn",         default_value="False"),
        DeclareLaunchArgument("robot_name",          default_value="ros2_cafe_robot"),
        DeclareLaunchArgument("world_file",          default_value="cafe.world"),
        DeclareLaunchArgument("headless",            default_value="False"),
        DeclareLaunchArgument("jsp_gui",             default_value="false"),
        DeclareLaunchArgument("load_controllers",    default_value="true"),
        DeclareLaunchArgument("use_gazebo",          default_value="true"),
        DeclareLaunchArgument("use_robot_state_pub", default_value="true"),
        DeclareLaunchArgument("use_rviz",            default_value="true"),
        DeclareLaunchArgument("use_sim_time",        default_value="true"),
        DeclareLaunchArgument("x",                   default_value="0.0"),
        DeclareLaunchArgument("y",                   default_value="0.0"),
        DeclareLaunchArgument("z",                   default_value="0.20"),
        DeclareLaunchArgument("roll",                default_value="0.0"),
        DeclareLaunchArgument("pitch",               default_value="0.0"),
        DeclareLaunchArgument("yaw",                 default_value="0.0"),
    ]

    # ── nodes / includes ─────────────────────────────────────────────────────

    cmd_vel_relay = Node(
        package="ros2_cafe_robot",
        executable="cmd_vel_relay",
        name="cmd_vel_relay",
        output="screen",
        parameters=[{"use_sim_time": LaunchConfiguration("use_sim_time")}],
    )

    ekf = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([LaunchConfiguration("ekf_launch_file")]),
        launch_arguments={
            "ekf_config_file": LaunchConfiguration("ekf_config_file"),
            "use_sim_time":    LaunchConfiguration("use_sim_time"),
        }.items(),
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([LaunchConfiguration("gazebo_launch_file")]),
        launch_arguments={
            "enable_odom_tf":      LaunchConfiguration("enable_odom_tf"),
            "headless":            LaunchConfiguration("headless"),
            "jsp_gui":             LaunchConfiguration("jsp_gui"),
            "load_controllers":    LaunchConfiguration("load_controllers"),
            "robot_name":          LaunchConfiguration("robot_name"),
            "rviz_config_file":    LaunchConfiguration("rviz_config_file"),
            "use_rviz":            LaunchConfiguration("use_rviz"),
            "use_gazebo":          LaunchConfiguration("use_gazebo"),
            "use_robot_state_pub": LaunchConfiguration("use_robot_state_pub"),
            "use_sim_time":        LaunchConfiguration("use_sim_time"),
            "world_file":          LaunchConfiguration("world_file"),
            "x":                   LaunchConfiguration("x"),
            "y":                   LaunchConfiguration("y"),
            "z":                   LaunchConfiguration("z"),
            "roll":                LaunchConfiguration("roll"),
            "pitch":               LaunchConfiguration("pitch"),
            "yaw":                 LaunchConfiguration("yaw"),
        }.items(),
    )

    nav2 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(nav2_launch_dir, "bringup_launch.py")),
        launch_arguments={
            "namespace":       LaunchConfiguration("namespace"),
            "use_namespace":   LaunchConfiguration("use_namespace"),
            "slam":            LaunchConfiguration("slam"),
            "map":             LaunchConfiguration("map"),
            "use_sim_time":    LaunchConfiguration("use_sim_time"),
            "params_file":     LaunchConfiguration("nav2_params_file"),
            "autostart":       LaunchConfiguration("autostart"),
            "use_composition": LaunchConfiguration("use_composition"),
            "use_respawn":     LaunchConfiguration("use_respawn"),
        }.items(),
    )

    # ── launch description ───────────────────────────────────────────────────

    ld = LaunchDescription(args)
    ld.add_action(cmd_vel_relay)
    ld.add_action(ekf)
    ld.add_action(gazebo)
    ld.add_action(nav2)
    return ld
