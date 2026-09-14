cmake_minimum_required(VERSION 3.8)
project(battery_lifecycle)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(
  battery_monitor_node
  src/battery_monitor_lifecycle.cpp
)

target_compile_features(
  battery_monitor_node
  PUBLIC cxx_std_17
)

ament_target_dependencies(
  battery_monitor_node
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  sensor_msgs
  std_msgs
)

install(
  TARGETS battery_monitor_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
