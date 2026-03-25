# ros2_cafe_robot

A ROS 2 differential drive robot simulated in Gazebo Harmonic, designed to navigate autonomously inside a cafe environment. The robot uses Nav2 for navigation, EKF for localization, MPPI controller for path following, and SLAM Toolbox for mapping.

---

## Demo

> Robot navigating in the cafe world with Nav2 and AMCL localization.

*(Add a GIF or screenshot here)*

---

## Features

- Differential drive robot with custom 3D mesh (humanoid-style waiter robot)
- Gazebo Harmonic simulation with LiDAR, IMU, and depth camera
- Extended Kalman Filter (EKF) fusing wheel odometry and IMU
- Nav2 full stack: AMCL, MPPI controller, NavFn planner, costmaps, behavior trees
- SLAM Toolbox support for mapping
- RViz2 visualization

---

## Package Structure

```
warehouse_system/
├── config/
│   ├── ekf.yaml                    # EKF sensor fusion config
│   ├── navigation_parameters.yaml  # Nav2 full stack parameters
│   ├── ros2_controllers.yaml       # Diff drive controller config
│   ├── ros_gz_bridge.yaml          # Gazebo <-> ROS 2 topic bridges
├── launch/
│   ├── navigation.launch.py        # Full system: Gazebo + EKF + Nav2
│   ├── robot_ekf.launch.py         # Gazebo + EKF only (no Nav2)
│   ├── robot_launch.launch.py      # Gazebo + robot only
│   ├── ekf_launch.launch.py        # EKF node only
│   ├── robot_state_publisher.launch.py
│   └── controllers.launch.py
├── maps/
│   └── cafe_world_map.yaml        # Pre-built map of the cafe world
│   └── cafe_world_map.pgm         # Photo for the map of the cafe world
├── mesh/                           # Robot 3D meshes (.dae)
├── model/
│   └── robot.xacro                 # Robot URDF/XACRO description
├── scripts/
│   ├── run_navigation.sh           # Quick launch: full navigation
│   └── run_ekf.sh                  # Quick launch: robot + EKF only
├── worlds/
│   ├── cafe.world                  # Cafe simulation environment
│   └── empty.world                 # Empty world For Testing 
├── CMakeLists.txt
└── package.xml
```

---

## Requirements

- ROS 2 Jazzy (or Humble)
- Gazebo Harmonic
- Nav2
- robot_localization
- ros_gz_bridge / ros_gz_sim
- gz_ros2_control
- slam_toolbox

---

## Build

```bash
cd ~/your_ws
colcon build --packages-select warehouse_system
source install/setup.bash
```

---

## Run

### Full navigation (with pre-built map)
```bash
cd src/warehouse_system/scripts
./run_navigation.sh
```

### Full navigation with SLAM (build map on the fly)
```bash
./run_navigation.sh slam
```

### Robot + EKF only (no Nav2)
```bash
./run_ekf.sh
```

### Manual launch
```bash
ros2 launch warehouse_system navigation.launch.py \
  world_file:=cafe.world \
  use_sim_time:=true \
  slam:=False \
  map:=/path/to/your/map.yaml
```

---

## TF Tree

```
map
 └── odom          (published by EKF)
      └── base_link
           ├── robot_body
           ├── left_wheel_link
           ├── right_wheel_link
           ├── caster_wheel_link
           ├── lidar_link
           └── imu_link
```

---

## Notes

- The `cmd_vel_relay` node converts `geometry_msgs/Twist` from Nav2 to `geometry_msgs/TwistStamped` required by the diff drive controller
- EKF fuses wheel odometry velocities (vx, vy, vyaw) and IMU yaw rate
- `enable_odom_tf` is set to `false` in the controller — the EKF handles the odom→base_link transform
- The cafe world ground plane has zero friction (`mu=0`) to prevent the robot from getting stuck on mesh edges

---

## Author

Ahmed — built as part of a robotics learning project.
