# ☕ Cafe Robot System (ROS 2)

An autonomous café service robot simulation built using ROS 2, Gazebo, Navigation2, and a real-time web interface.

---

## 🚀 Project Overview

This project represents a complete robotic system designed to simulate an autonomous café service robot.

The robot is capable of:

* Navigating autonomously inside a café environment
* Receiving and managing multiple orders
* Delivering items efficiently based on optimal routing
* Streaming live camera feed
* Providing real-time status updates through a web interface

---

## 🧠 System Architecture

The system is divided into three main packages:

* **robot_cafe_sim**

  * Robot modeling (URDF/Xacro)
  * Gazebo simulation
  * Sensors integration
  * Navigation (Nav2)
  * EKF localization

* **robot_manager**

  * Task management logic
  * Order handling system
  * Robot decision making
  * Services & communication

* **robot_manager_interface**

  * Web dashboard (Flask)
  * ROS integration
  * Live robot monitoring
  * Camera streaming

---

## 🤖 Robot Modeling (URDF/Xacro)

* Imported base model from GrabCAD
* Defined all links and joints
* Structured using Xacro for flexibility
* Added sensor frames and mounting points

---

## 🌍 Simulation (Gazebo)

A complete café environment was created including:

* 6 customer tables
* Kitchen area
* Charging station

### 🔧 Sensors Integrated:

* LiDAR (for mapping & navigation)
* Camera (RGB image stream)
* IMU (orientation and motion)

Each sensor is connected using Gazebo plugins and ROS bridges.

---

## ⚙️ ROS2 Control

The robot is controlled using:

* `diff_drive_controller` → movement
* `joint_state_broadcaster` → state publishing

---

## 🧭 Localization & Sensor Fusion

Implemented **Extended Kalman Filter (EKF)** to fuse:

* IMU data
* Odometry data

Generated a stable topic:

```
/odometry/filtered
```

This is used in both:

* SLAM
* Navigation

---

## 🗺️ Navigation (Nav2)

* Configured Navigation2 stack
* Tuned parameters based on robot dimensions
* Implemented AMCL auto-initial pose (no manual reset)

---

## 📦 Task Management System

A custom system was built to simulate real café operations:

* Robot waits in kitchen until release
* Can carry up to 3 orders
* Chooses nearest destination dynamically
* Handles:

  * Preparing orders
  * Ready orders
  * Delivered orders
* Simulated battery behavior
* Tracks delivery statistics

---

## 🌐 Web Interface (Flask)

A full dashboard was developed:

### Features:

* Real-time robot status
* Place new orders
* Release robot from kitchen
* View robot state & metrics
* Live camera streaming from ROS topic

### Technologies Used:

* Flask (Backend)
* ROS 2 (Integration)
* OpenCV + cv_bridge (Camera streaming)

---

## 📸 Demo

> 📌 Add screenshots here:

* Gazebo simulation
* RViz navigation
* Robot model
* Web interface

---

## 🎥 Demo Video

> 📌 Add demo video link here

---

## 🛠️ Installation

```bash
cd ~/your_ws
git clone <your-repo>
colcon build
source install/setup.bash
```

---

## ⚠️ Important (Gazebo Setup)

To load models and worlds correctly, you must export:

```bash
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/your_ws/src
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/your_ws/src/robot_cafe_sim
```

To make it permanent:

```bash
echo 'export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/your_ws/src' >> ~/.bashrc
echo 'export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/your_ws/src/robot_cafe_sim' >> ~/.bashrc
source ~/.bashrc
```

---

## ▶️ Usage

### 1️⃣ Run SLAM (First Time Only)

```bash
ros2 launch robot_cafe_bringup bringup.launch.py slam_mode:=true
```

Save the generated map.

---

### 2️⃣ Run Full System

```bash
ros2 launch robot_cafe_bringup bringup.launch.py
```

---

## 🌐 Web Interface

The interface will open automatically:

```
http://localhost:5000
```

---

## 🧪 What You Can Do

* Send orders to robot
* Monitor robot status
* Watch live camera feed
* Track deliveries
* Simulate real café workflow

---

## 🧠 Skills Demonstrated

* ROS 2 (Nodes, Topics, Services)
* Gazebo Simulation
* URDF/Xacro Modeling
* Navigation2 (Nav2)
* Sensor Fusion (EKF)
* Path Planning & Task Logic
* Python + Flask Integration
* OpenCV + ROS Image Processing

---

## 👨‍💻 Author

Ahmed Hassan
🔗 LinkedIn: https://www.linkedin.com/in/eng-ahmed-hassan-ah6100/
📧 Email: *(add your email here)*

---

## 🙏 Credits

> 📌 Add GrabCAD model link here
