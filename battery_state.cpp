#include <cmath>
#include <iostream>
#include <optional>
#include <string_view>
#include <limits>

enum class BatteryError
{
    SensorDisconnected,
    InvalidReading,
    OutOfRange
};

// 1. Implementa as strings para cada erro.
std::string_view toString(BatteryError error)
{
    switch (error)
    {
    case BatteryError::SensorDisconnected:
        return std::string_view("sensor is disconnectd");

    case BatteryError::InvalidReading:
        return std::string_view("reading is invalid");

    case BatteryError::OutOfRange:
        return std::string_view("reading is out of range");

    default:
        return std::string_view("unknown error");
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

int main()
{
    BatteryMonitor monitor;

    const auto valid = monitor.validateReading(74.5, true);
    const auto disconnected = monitor.validateReading(50.0, false);
    const auto invalid = monitor.validateReading(
        std::numeric_limits<double>::quiet_NaN(),
        true);
    const auto out_of_range = monitor.validateReading(105.0, true);

    const auto print_result = [](const Result<double, BatteryError> &result)
    {
        if (result.hasValue())
        {
            std::cout << "Battery: " << result.value() << "%\n";
        }
        else
        {
            std::cout << "Battery error: "
                      << toString(result.error())
                      << '\n';
        }
    };

    print_result(valid);
    print_result(disconnected);
    print_result(invalid);
    print_result(out_of_range);
}