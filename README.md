# ☕ Cafe Robot System (ROS 2)

![ROS2](https://img.shields.io/badge/ROS2-Jazzy-blue)
![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange)
![Nav2](https://img.shields.io/badge/Nav2-Navigation-success)
![Python](https://img.shields.io/badge/Python-3.x-yellow)
![Flask](https://img.shields.io/badge/Flask-Web%20Interface-black)
![Status](https://img.shields.io/badge/Project-Completed-brightgreen)

An autonomous café service robot simulation built using **ROS 2**, **Gazebo**, **Navigation2**, and a **real-time web interface**.

---

## 🎥 Demo

> Put your demo video link here.

[![Watch the demo](PUT_VIDEO_THUMBNAIL_IMAGE_HERE)](PUT_VIDEO_LINK_HERE)

If you do not have a thumbnail yet, use a normal link:

**Demo Video:** PUT_VIDEO_LINK_HERE

---

## 📸 System Preview

> Replace these placeholders with your real screenshots.

### Gazebo Simulation

![Gazebo Simulation](ADD_GAZEBO_SCREENSHOT_HERE)

### RViz Navigation

![RViz Navigation](ADD_RVIZ_SCREENSHOT_HERE)

### Robot Model

![Robot Model](ADD_ROBOT_MODEL_SCREENSHOT_HERE)

### Web Interface

![Web Interface](ADD_WEB_INTERFACE_SCREENSHOT_HERE)

---

## 🚀 Project Overview

This project simulates a complete robotic system for an autonomous café service robot.

The robot is capable of:

* navigating autonomously inside a café environment
* receiving and managing customer orders
* delivering orders efficiently
* tracking robot status in real time
* streaming live camera feed to a web interface
* simulating a real café workflow with kitchen, tables, and charging area

---

## 🧠 System Architecture

The project is divided into three main ROS 2 packages:

* **robot_cafe_sim**

  * robot description using URDF/Xacro
  * Gazebo simulation
  * sensors and plugins
  * EKF localization
  * Navigation2 integration

* **robot_manager**

  * task management logic
  * order handling
  * delivery decision making
  * robot state publishing
  * service definitions and custom messages

* **robot_manager_interface**

  * Flask-based web dashboard
  * ROS 2 integration
  * live robot monitoring
  * order submission
  * live camera feed display

* **robot_cafe_bringup**

  * launches the full system

---

## 📁 Workspace Tree

Below is the main workspace structure:

```text
ros2_cafe_robot/
├── src/
│   ├── robot_cafe_bringup/
│   │   ├── include/
│   │   ├── launch/
│   │   │   └── bringup.launch.py
│   │   ├── CMakeLists.txt
│   │   └── package.xml
│   │
│   ├── robot_cafe_sim/
│   │   ├── config/
│   │   │   ├── ekf.yaml
│   │   │   ├── navigation_parameters.yaml
│   │   │   ├── ros_gz_bridge.yaml
│   │   │   └── ros2_controllers.yaml
│   │   ├── launch/
│   │   │   ├── controllers.launch.py
│   │   │   ├── ekf_launch.launch.py
│   │   │   ├── navigation.launch.py
│   │   │   ├── robot_ekf.launch.py
│   │   │   ├── robot_launch.launch.py
│   │   │   └── robot_state_publisher.py
│   │   ├── maps/
│   │   │   ├── cafe_map.pgm
│   │   │   └── cafe_map.yaml
│   │   ├── mesh/
│   │   │   ├── foodplate_modified.dae
│   │   │   ├── left_arm_modified.dae
│   │   │   ├── right_arm_modified.dae
│   │   │   ├── robot_body_modified.dae
│   │   │   └── robot_head_modified.dae
│   │   ├── model/
│   │   │   └── robot.xacro
│   │   ├── models/
│   │   ├── rviz/
│   │   │   ├── nav2_default_view.rviz
│   │   │   └── rviz_parameters.rviz
│   │   ├── Screens/
│   │   ├── scripts/
│   │   │   ├── run_ekf.sh
│   │   │   └── run_navigation.sh
│   │   ├── src/
│   │   │   └── cmd_vel_relay.cpp
│   │   ├── worlds/
│   │   │   ├── cafe.world
│   │   │   └── empty.world
│   │   ├── CMakeLists.txt
│   │   └── package.xml
│   │
│   ├── robot_manager/
│   │   ├── include/
│   │   ├── launch/
│   │   │   └── manager.launch.py
│   │   ├── msg/
│   │   │   └── RobotStatus.msg
│   │   ├── src/
│   │   │   └── cafe_task_manager.cpp
│   │   ├── srv/
│   │   │   └── AddOrder.srv
│   │   ├── CMakeLists.txt
│   │   └── package.xml
│   │
│   └── robot_manager_interface/
│       ├── launch/
│       │   └── interface.launch.py
│       ├── resource/
│       │   └── robot_manager_interface
│       ├── robot_manager_interface/
│       │   ├── __init__.py
│       │   └── web_bridge.py
│       ├── static/
│       │   ├── app.js
│       │   └── style.css
│       ├── templates/
│       │   └── index.html
│       ├── test/
│       ├── package.xml
│       ├── setup.cfg
│       └── setup.py
│
├── build/
├── install/
└── log/
```

---

## 🤖 Robot Modeling

The robot was modeled using **URDF/Xacro**.

### What was done:

* imported a base model from **GrabCAD**
* defined robot links and joints
* organized the robot structure in `robot.xacro`
* added dedicated links for sensors
* prepared the model for simulation and control integration

---

## 🌍 Gazebo Simulation

A custom café environment was created in Gazebo.

### The world includes:

* 6 café tables
* a kitchen area
* a charging area

### Gazebo integration includes:

* world setup
* physics configuration
* sensor plugins
* ROS-Gazebo bridge integration
* ROS 2 Control integration

---

## 📡 Sensors Used

The robot uses the following sensors:

* **LiDAR**
* **IMU**
* **RGB Camera**

These sensors were integrated into Gazebo and connected to ROS 2 topics for downstream use in localization, navigation, and monitoring.

---

## ⚙️ ROS 2 Control

The robot was connected to **ROS 2 Control** and configured with:

* `diff_drive_controller`
* `joint_state_broadcaster`

This allows the robot to move properly in simulation and publish required joint information.

---

## 🧭 Localization and Sensor Fusion

An **Extended Kalman Filter (EKF)** was added to fuse sensor information and produce a filtered odometry topic:

```text
/odometry/filtered
```

This filtered odometry is used later in:

* SLAM
* Navigation

---

## 🗺️ Navigation

The project uses **Navigation2 (Nav2)**.

### Navigation work included:

* tuning navigation parameters to match the robot dimensions and behavior
* integrating localization with EKF
* setting up the robot initial pose logic using AMCL so the starting position does not need to be manually initialized each time

---

## 📦 Task Management Logic

A higher-level task manager was implemented in `cafe_task_manager`.

### Main logic:

* the robot starts from the kitchen
* when an order is received, the robot waits until it is released
* the robot can carry up to **3 orders**
* the robot chooses the **nearest next target**
* table coordinates, kitchen, and charging location are predefined
* battery state is simulated
* delivery counts and robot statistics are tracked

### Status handling includes:

* preparing orders
* ready orders
* onboard orders
* delivered orders
* waiting state
* low battery mode

---

## 🌐 Web Interface

A web dashboard was built using **Flask** and integrated with ROS 2.

### Features:

* place customer orders
* monitor robot state in real time
* release the robot from the kitchen
* view robot statistics
* display live camera feed from the robot

### ROS integration:

* subscribes to `robot_status`
* uses OpenCV and `cv_bridge` to display camera data in the dashboard

This makes the project feel closer to a real robot operations interface instead of a simulation-only demo.

---

## 🛠️ Installation

Clone the workspace and build:

```bash
cd ~/ros2_cafe_robot
colcon build
source install/setup.bash
```

---

## ⚠️ Important Gazebo Setup

Before running the project, make sure Gazebo can find the robot and world resources.

Export these paths:

```bash
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/ros2_cafe_robot/src
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/ros2_cafe_robot/src/robot_cafe_sim
```

To make this permanent:

```bash
echo 'export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/ros2_cafe_robot/src' >> ~/.bashrc
echo 'export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/ros2_cafe_robot/src/robot_cafe_sim' >> ~/.bashrc
source ~/.bashrc
```

Without this, Gazebo may fail to load:

* the robot model
* custom world models
* referenced meshes and resources

---

## ▶️ Usage

### 1. Run SLAM (First Time Only)

If you do not have a map yet, start the system in SLAM mode:

```bash
ros2 launch robot_cafe_bringup bringup.launch.py slam_mode:=true
```

Then save the generated map.

---

### 2. Run the Full System

After a map has already been created, run the full system normally:

```bash
ros2 launch robot_cafe_bringup bringup.launch.py
```

---

## 📌 Important Note About Bringup

The normal bringup mode assumes that a valid map already exists.

### Recommended workflow:

1. Run SLAM first to create the map
2. Save the map
3. Run the normal bringup launch for autonomous navigation and task handling

---

## 🌐 Web Interface

The web interface opens automatically when the full system starts.

Default address:

```text
http://localhost:5000
```

---

## 🧪 What You Can Do

With the full system running, you can:

* send orders from the web dashboard
* monitor robot status live
* track delivery progress
* watch the live camera feed
* simulate a real café workflow

---

## 🧠 Skills Demonstrated

This project demonstrates practical experience in:

* ROS 2
* Gazebo simulation
* URDF/Xacro modeling
* ROS 2 Control
* Navigation2 (Nav2)
* EKF sensor fusion
* task planning and robot logic
* Flask web integration
* OpenCV and ROS image processing

---

## 📷 Suggested Media to Add

To make the repository more attractive, add:

### Screenshots

* Gazebo world overview
* RViz with robot and navigation path
* web dashboard screenshot
* close-up of the robot model

### Video

A short demo video showing:

* system startup
* placing an order from the web interface
* robot leaving the kitchen
* robot delivering to a table
* live camera feed on the web interface

---

## 👨‍💻 Author

**Ahmed Hassan**
LinkedIn: https://www.linkedin.com/in/eng-ahmed-hassan-ah6100/
Email: ADD_YOUR_EMAIL_HERE

---

## 🙏 Credits

GrabCAD model credit: ADD_MODEL_LINK_HERE
