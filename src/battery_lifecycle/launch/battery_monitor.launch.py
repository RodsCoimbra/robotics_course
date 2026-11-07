from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    params_file_argument = DeclareLaunchArgument(
        "params_file",
        default_value="",
        description="Path to the ROS 2 parameters YAML file.",
    )
    
    params_file = LaunchConfiguration("params_file")
    
    battery_monitor_node = Node(
        package="battery_lifecycle",
        executable="battery_monitor_node",
        name="battery_monitor",
        output="screen",
        parameters=[params_file]
    )
    
    return LaunchDescription([
        params_file_argument,
        battery_monitor_node
    ])