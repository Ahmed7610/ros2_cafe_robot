cmake_minimum_required(VERSION 3.8)
project(robot_cafe_sim)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(cmd_vel_relay src/cmd_vel_relay.cpp)
ament_target_dependencies(cmd_vel_relay
  rclcpp
  geometry_msgs
)

install(TARGETS
  cmd_vel_relay
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY
    launch
    model
    config
    rviz
    worlds
    models
    mesh
    maps
    scripts
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
