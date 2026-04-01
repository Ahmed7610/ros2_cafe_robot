from launch import LaunchDescription
from launch.actions import ExecuteProcess, RegisterEventHandler, TimerAction
from launch.event_handlers import OnProcessExit


def generate_launch_description():
    joint_state_broadcaster = ExecuteProcess(
        cmd=[
            "ros2", "control", "load_controller",
            "--set-state", "active",
            "joint_state_broadcaster"
        ],
        output="screen"
    )

    diff_drive_controller = ExecuteProcess(
        cmd=[
            "ros2", "control", "load_controller",
            "--set-state", "active",
            "diff_drive_controller"
        ],
        output="screen"
    )

    delayed_joint_state_broadcaster = TimerAction(
        period=5.0,
        actions=[joint_state_broadcaster]
    )

    load_diff_drive_after_jsb = RegisterEventHandler(
        OnProcessExit(
            target_action=joint_state_broadcaster,
            on_exit=[diff_drive_controller]
        )
    )

    ld = LaunchDescription()
    ld.add_action(delayed_joint_state_broadcaster)
    ld.add_action(load_diff_drive_after_jsb)
    return ld
