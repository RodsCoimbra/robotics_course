#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
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

class BatteryMonitorNode : public rclcpp::Node
{
public:
    BatteryMonitorNode() : Node("battery_monitor")
    {
        // 1. Cria a subscription para "/battery_state".
        // Escolhe um QoS para estado atual de bateria.
        battery_subscription_ = create_subscription<sensor_msgs::msg::BatteryState>(
            "/battery_state",
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable(),
            std::bind(&BatteryMonitorNode::batteryCallback, this, std::placeholders::_1));

        status_publisher_ = create_publisher<std_msgs::msg::String>(
            "/battery_monitor/status",
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable().durability_volatile());
        // Only the last reading is important, reliable as this reading are usually slow and its okay to be reliable
        // Volatile as it isn't so important to get previous measurments when connected
    }

private:
    // 3. Escolhe qual o campo BatteryState que representa percentagem.
    // 4. Decide como inferir sensor_connected neste exercício.
    // 5. Chama battery_monitor_.validateReading(...).
    // 6. Cria e publica std_msgs::msg::String.
    // 7. Se houver erro, usa RCLCPP_WARN ou RCLCPP_ERROR;
    //    não deixes uma exceção escapar do callback.
    void batteryCallback(
        const sensor_msgs::msg::BatteryState::SharedPtr message)
    {
        Result<double, BatteryError> result = battery_monitor_.validateReading(static_cast<double>(message->percentage) * 100.0, message->present);
        if (result.hasValue())
        {
            std_msgs::msg::String msg;
            msg.data = std::to_string(result.value());
            status_publisher_->publish(msg);
            return;
        }

        RCLCPP_WARN(this->get_logger(), "Battery msg had an error: %s", toString(result.error()));
    }

    BatteryMonitor battery_monitor_;
    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery_subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_publisher_;
};

int main(int argc, char const *argv[])
{
    rclcpp::init(argc, argv);
    auto battery_node = std::make_shared<BatteryMonitorNode>();
    rclcpp::spin(battery_node);
    rclcpp::shutdown();
    return 0;
}
