#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <lifecycle_msgs/msg/state.hpp>
#include <lifecycle_msgs/msg/transition.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <sensor_msgs/msg/battery_state.hpp>
#include <std_msgs/msg/string.hpp>

class BatteryMonitorLifecycleNode
    : public rclcpp_lifecycle::LifecycleNode
{
public:
    BatteryMonitorLifecycleNode()
        : LifecycleNode("battery_monitor")
    {
        declare_parameter<std::string>(
            "battery_topic",
            "/battery_state");

        declare_parameter<std::string>(
            "status_topic",
            "/battery_monitor/status");
    }

private:
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::
            CallbackReturn;

    CallbackReturn on_configure(
        const rclcpp_lifecycle::State &)
    {
        battery_topic_ = get_parameter("battery_topic").as_string();
        status_topic_ = get_parameter("status_topic").as_string();

        if (battery_topic_.empty() || status_topic_.empty())
        {
            RCLCPP_ERROR(
                get_logger(),
                "Battery and status topics must not be empty.");

            return CallbackReturn::FAILURE;
        }

        const auto battery_qos =
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable();

        battery_subscription_ =
            create_subscription<sensor_msgs::msg::BatteryState>(
                battery_topic_,
                battery_qos,
                std::bind(
                    &BatteryMonitorLifecycleNode::batteryCallback,
                    this,
                    std::placeholders::_1));

        const auto status_qos =
            rclcpp::QoS(rclcpp::KeepLast(1))
                .reliable()
                .transient_local();

        status_publisher_ =
            create_publisher<std_msgs::msg::String>(
                status_topic_,
                status_qos);

        RCLCPP_INFO(
            get_logger(),
            "Configured battery monitor. Input: '%s', output: '%s'.",
            battery_topic_.c_str(),
            status_topic_.c_str());

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(
        const rclcpp_lifecycle::State &)
    {
        status_publisher_->on_activate();

        RCLCPP_INFO(
            get_logger(),
            "Battery monitor activated.");

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(
        const rclcpp_lifecycle::State &)
    {
        status_publisher_->on_deactivate();

        RCLCPP_INFO(
            get_logger(),
            "Battery monitor deactivated.");

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(
        const rclcpp_lifecycle::State &)
    {
        battery_subscription_.reset();
        status_publisher_.reset();

        battery_topic_.clear();
        status_topic_.clear();

        RCLCPP_INFO(
            get_logger(),
            "Battery monitor cleaned up.");

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(
        const rclcpp_lifecycle::State &)
    {
        battery_subscription_.reset();
        status_publisher_.reset();

        RCLCPP_INFO(
            get_logger(),
            "Battery monitor shut down.");

        return CallbackReturn::SUCCESS;
    }

    void batteryCallback(
        const sensor_msgs::msg::BatteryState::SharedPtr message)
    {
        if (get_current_state().id() !=
            lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE)
        {
            return;
        }

        const double battery_percent =
            static_cast<double>(message->percentage) * 100.0;

        std_msgs::msg::String status;

        if (!message->present)
        {
            status.data = "Battery error: battery not present";

            RCLCPP_WARN_THROTTLE(
                get_logger(),
                *get_clock(),
                5000,
                "Battery is not present.");
        }
        else if (!std::isfinite(battery_percent))
        {
            status.data = "Battery error: invalid reading";

            RCLCPP_WARN_THROTTLE(
                get_logger(),
                *get_clock(),
                5000,
                "Battery percentage is not finite.");
        }
        else if (battery_percent < 0.0 || battery_percent > 100.0)
        {
            status.data = "Battery error: reading out of range";

            RCLCPP_WARN_THROTTLE(
                get_logger(),
                *get_clock(),
                5000,
                "Battery percentage %.2f is out of range.",
                battery_percent);
        }
        else
        {
            status.data =
                "Battery: " + std::to_string(battery_percent) + "%";
        }

        status_publisher_->publish(status);
    }

    std::string battery_topic_;
    std::string status_topic_;

    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr
        battery_subscription_;

    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr
        status_publisher_;
};