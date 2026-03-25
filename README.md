# ☕ ROS2 Autonomous Cafe Robot

A ROS2-based autonomous mobile robot designed to simulate real-world service scenarios in a café environment, focusing on navigation reliability, sensor fusion, and system integration.

---

## 🚀 Project Overview

This project implements a full robotics pipeline, from robot modeling to autonomous navigation, with an emphasis on building a system that behaves consistently rather than just functionally.

---

## ⚙️ Core Capabilities

- Custom robot modeling using URDF
- Sensor integration (LiDAR, IMU)
- Motion control using ros2_control (diff drive)
- EKF-based sensor fusion for stable odometry
- SLAM-based mapping
- Autonomous navigation using Navigation2
- Command bridging between navigation and low-level controller
- Realistic café simulation environment in Gazebo

---

## 🏗️ System Architecture

![Architecture](Screens/architecture.png)

The system is structured into:

- **Perception** → LiDAR, IMU
- **Localization** → EKF sensor fusion
- **Planning** → SLAM & Navigation2
- **Control** → ros2_control & motor interface

---

## 🎥 System Demonstration

### Mapping (SLAM)
![SLAM](Screens/slam.png)

### Autonomous Navigation
![Navigation](Screens/navigation.png)

### System Visualization (RViz)
![RViz](Screens/rviz.png)

---

## 🔍 Engineering Insights

- Designed a consistent command flow from Navigation2 to low-level control
- Improved odometry stability using EKF fusion of IMU and encoder data
- Handled TF conflicts by isolating odometry sources
- Built a modular pipeline that reflects real-world robotics system design

---

## 🛠️ Tech Stack

- ROS2
- Python
- Navigation2
- robot_localization (EKF)
- Gazebo
- RViz

---

## 📬 Contact

**Ahmed Hassan**  

[![Email](https://img.shields.io/badge/Email-engahmedhassan309%40gmail.com-red?style=flat&logo=gmail)](mailto:engahmedhassan309@gmail.com)  
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Ahmed%20Hassan-blue?style=flat&logo=linkedin)](https://www.linkedin.com/in/eng-ahmed-hassan-ah6100/)
