#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/executors/multi_threaded_executor.hpp>
#include <sensor_msgs/msg/image.hpp>

class ObjectDetectionNode : public rclcpp::Node
{
public:
  ObjectDetectionNode()
      : Node("object_detection")
  {
    // 1. Criar callback groups.
    image_callback_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    watchdog_callback_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    // 2. Criar subscription para Image.
    const auto image_qos = rclcpp::SensorDataQoS().keep_last(1);
    rclcpp::SubscriptionOptions sub_options;
    sub_options.callback_group = image_callback_group_;
    image_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
        "image_topic", image_qos, std::bind(&ObjectDetectionNode::imageCallback, this, std::placeholders::_1),
        sub_options);
    // 3. Associar subscription ao grupo correto.
    // 4. Criar timer de watchdog.
    watchdog_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&ObjectDetectionNode::watchdogCallback, this), watchdog_callback_group_);
    // 5. Associar timer ao grupo correto.
  }

private:
  void imageCallback(
      const sensor_msgs::msg::Image::SharedPtr)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  void watchdogCallback()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Declarar callback groups.
  rclcpp::CallbackGroup::SharedPtr image_callback_group_;
  rclcpp::CallbackGroup::SharedPtr watchdog_callback_group_;
  // Declarar subscription.
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscription_;
  // Declarar timer.
  rclcpp::TimerBase::SharedPtr watchdog_timer_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ObjectDetectionNode>();
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 2);
  executor.add_node(node->get_node_base_interface());
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
