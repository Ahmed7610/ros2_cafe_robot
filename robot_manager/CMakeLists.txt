cmake_minimum_required(VERSION 3.8)
project(robot_manager)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_default_runtime REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/AddOrder.srv"
  "msg/RobotStatus.msg"
  DEPENDENCIES std_msgs
)

add_executable(cafe_task_manager src/cafe_task_manager.cpp)

ament_target_dependencies(cafe_task_manager
  rclcpp
  rclcpp_action
  geometry_msgs
  nav2_msgs
  std_srvs
  tf2
)

rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME}
  "rosidl_typesupport_cpp"
)

target_link_libraries(cafe_task_manager
  ${cpp_typesupport_target}
)

install(TARGETS
  cafe_task_manager
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
