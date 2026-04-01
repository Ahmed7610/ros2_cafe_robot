from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare("robot_cafe_sim")

    urdf_model = LaunchConfiguration("urdf_model")
    rviz_config_file = LaunchConfiguration("rviz_config_file")
    jsp_gui = LaunchConfiguration("jsp_gui")
    use_jsp = LaunchConfiguration("use_jsp")
    use_rviz = LaunchConfiguration("use_rviz")
    use_sim_time = LaunchConfiguration("use_sim_time")
    robot_name = LaunchConfiguration("robot_name")
    prefix = LaunchConfiguration("prefix")
    use_gazebo = LaunchConfiguration("use_gazebo")

    declared_arguments = [
        DeclareLaunchArgument(
            "robot_name",
            default_value="CafeRobot",
            description="Robot name"
        ),
        DeclareLaunchArgument(
            "prefix",
            default_value="",
            description="Prefix for joints/links"
        ),
        DeclareLaunchArgument(
            "use_gazebo",
            default_value="true",
            choices=["true", "false"]
        ),
        DeclareLaunchArgument(
            "jsp_gui",
            default_value="false",
            choices=["true", "false"]
        ),
        DeclareLaunchArgument(
            "use_jsp",
            default_value="false",
            choices=["true", "false"]
        ),
        DeclareLaunchArgument(
            "use_rviz",
            default_value="true",
            choices=["true", "false"]
        ),
        DeclareLaunchArgument(
            "use_sim_time",
            default_value="true",
            choices=["true", "false"]
        ),
        DeclareLaunchArgument(
            "urdf_model",
            default_value=PathJoinSubstitution(
                [pkg_share, "model", "robot.xacro"]
            ),
            description="Absolute path to robot xacro file"
        ),
        DeclareLaunchArgument(
            "rviz_config_file",
            default_value=PathJoinSubstitution(
                [pkg_share, "rviz", "rviz_parameters.rviz"]
            ),
            description="RViz config file"
        ),
    ]

    robot_description = ParameterValue(
        Command([
            "xacro", " ", urdf_model,
            " ", "robot_name:=", robot_name,
            " ", "prefix:=", prefix,
            " ", "use_gazebo:=", use_gazebo
        ]),
        value_type=str
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{
            "use_sim_time": use_sim_time,
            "robot_description": robot_description
        }]
    )

    joint_state_publisher = Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
        name="joint_state_publisher",
        output="screen",
        parameters=[{"use_sim_time": use_sim_time}],
        condition=IfCondition(use_jsp)
    )

    joint_state_publisher_gui = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        name="joint_state_publisher_gui",
        output="screen",
        parameters=[{"use_sim_time": use_sim_time}],
        condition=IfCondition(jsp_gui)
    )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", rviz_config_file],
        parameters=[{"use_sim_time": use_sim_time}],
        condition=IfCondition(use_rviz)
    )

    ld = LaunchDescription()
    for arg in declared_arguments:
        ld.add_action(arg)

    ld.add_action(joint_state_publisher)
    ld.add_action(joint_state_publisher_gui)
    ld.add_action(robot_state_publisher)
    ld.add_action(rviz)

    return ld
