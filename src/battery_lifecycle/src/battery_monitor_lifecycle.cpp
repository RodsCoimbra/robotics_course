#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/transition.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <sensor_msgs/msg/battery_state.hpp>
#include <std_msgs/msg/string.hpp>
#include <cmath>
#include <iostream>
#include <optional>
#include <limits>
#include <functional>
#include <utility>

enum class BatteryError
{
    SensorDisconnected,
    InvalidReading,
    OutOfRange
};

// 1. Implementa as strings para cada erro.
const char *toString(BatteryError error)
{
    switch (error)
    {
    case BatteryError::SensorDisconnected:
        return "sensor disconnected";

    case BatteryError::InvalidReading:
        return "invalid reading";

    case BatteryError::OutOfRange:
        return "reading is out of range";

    default:
        return "unknown error";
    }
}

template <typename T, typename E>
class Result
{
public:
    static Result success(T value)
    {
        return Result(std::move(value));
    }

    static Result failure(E error)
    {
        return Result(error);
    }

    bool hasValue() const
    {
        return value_.has_value();
    }

    const T &value() const
    {
        return *value_;
    }

    E error() const
    {
        return error_;
    }

private:
    explicit Result(T value)
        : value_(std::move(value))
    {
    }

    explicit Result(E error)
        : error_(error)
    {
    }

    std::optional<T> value_;
    E error_{};
};

class BatteryMonitor
{
public:
    Result<double, BatteryError> validateReading(double battery_percent, bool sensor_connected) const
    {
        if (!sensor_connected)
            return Result<double, BatteryError>::failure(BatteryError::SensorDisconnected);

        if (!std::isfinite(battery_percent))
            return Result<double, BatteryError>::failure(BatteryError::InvalidReading);

        if (!(0.0 <= battery_percent && battery_percent <= 100.0))
            return Result<double, BatteryError>::failure(BatteryError::OutOfRange);

        return Result<double, BatteryError>::success(battery_percent);
    }
};

class BatteryMonitorLifecycleNode : public rclcpp_lifecycle::LifecycleNode
{
public:
    BatteryMonitorLifecycleNode() : LifecycleNode("battery_monitor")
    {
        declare_parameter<std::string>(
            "battery_topic",
            "/battery_state");
        declare_parameter<std::string>(
            "status_topic",
            "/battery_monitor/status");
        declare_parameter<double>(
            "low_battery_threshold",
            20.0);

        declare_parameter<double>(
            "critical_battery_threshold",
            10.0);

        declare_parameter<int>(
            "status_qos_depth",
            1);

        declare_parameter<bool>(
            "publish_transient_local",
            true);
    }

private:
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

    CallbackReturn on_configure(const rclcpp_lifecycle::State &)
    {
        battery_topic = get_parameter("battery_topic").as_string();
        status_topic = get_parameter("status_topic").as_string();
        low_battery_threshold_ = get_parameter("low_battery_threshold").as_double();
        critical_battery_threshold_ = get_parameter("critical_battery_threshold").as_double();
        const int status_qos_depth = get_parameter("status_qos_depth").as_int();
        const bool publish_transient_local = get_parameter("publish_transient_local").as_bool();
        if (battery_topic.empty() || status_topic.empty())
        {
            RCLCPP_ERROR(this->get_logger(), "One of the topic was empty.");
            return CallbackReturn::FAILURE;
        }

        if (!std::isfinite(low_battery_threshold_) ||
            !std::isfinite(critical_battery_threshold_))
        {
            RCLCPP_ERROR(
                get_logger(),
                "Battery thresholds must be finite.");

            return CallbackReturn::FAILURE;
        }

        if (critical_battery_threshold_ < 0.0 ||
            low_battery_threshold_ > 100.0 ||
            critical_battery_threshold_ > low_battery_threshold_)
        {
            RCLCPP_ERROR(
                get_logger(),
                "Thresholds must satisfy: "
                "0 <= critical <= low <= 100.");

            return CallbackReturn::FAILURE;
        }

        if (status_qos_depth <= 0)
        {
            RCLCPP_ERROR(
                get_logger(),
                "status_qos_depth must be greater than zero.");

            return CallbackReturn::FAILURE;
        }

        battery_subscription_ = create_subscription<sensor_msgs::msg::BatteryState>(
            battery_topic,
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable(),
            std::bind(&BatteryMonitorLifecycleNode::batteryCallback, this, std::placeholders::_1));

        status_publisher_ = create_publisher<std_msgs::msg::String>(
            status_topic,
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local());

        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &)
    {
        status_publisher_->on_activate();
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &)
    {
        status_publisher_->on_deactivate();
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
    {
        status_publisher_.reset();
        battery_subscription_.reset();
        battery_topic.clear();
        status_topic.clear();
        return CallbackReturn::SUCCESS;
    }

    void batteryCallback(const sensor_msgs::msg::BatteryState::SharedPtr message)
    {
        if (get_current_state().id() != lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE)
        {
            return;
        }

        Result<double, BatteryError> result = battery_monitor_.validateReading(static_cast<double>(message->percentage) * 100.0, message->present);
        std_msgs::msg::String status_message;

        if (result.hasValue())
        {
            status_message.data =
                "Battery: " + std::to_string(result.value()) + "%";

            RCLCPP_DEBUG(
                get_logger(),
                "Battery reading accepted: %.2f%%",
                result.value());
        }
        else
        {
            const char *error_text = toString(result.error());

            status_message.data =
                std::string("Battery error: ") + error_text;

            RCLCPP_WARN(
                get_logger(),
                "Battery message rejected: %s",
                error_text);
        }

        status_publisher_->publish(status_message);
    }

    std::string battery_topic;
    std::string status_topic;
    double low_battery_threshold_{20.0};
    double critical_battery_threshold_{10.0};
    BatteryMonitor battery_monitor_;
    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery_subscription_;
    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr status_publisher_;
};

int main(int argc, char const *argv[])
{
    rclcpp::init(argc, argv);
    auto battery_node = std::make_shared<BatteryMonitorLifecycleNode>();
    rclcpp::spin(battery_node->get_node_base_interface());
    rclcpp::shutdown();
    return 0;
}
