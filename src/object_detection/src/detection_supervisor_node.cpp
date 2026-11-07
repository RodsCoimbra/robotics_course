#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <std_srvs/srv/set_bool.hpp>
#include <chrono>

class DetectionSupervisorNode : public rclcpp::Node
{
public:
    DetectionSupervisorNode() : Node("detection_supervisor")
    {
        client_ = this->create_client<SetBool>("/set_detection_enabled");
        timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&DetectionSupervisorNode::send_deactivate, this));
    }

private:
    using SetBool = std_srvs::srv::SetBool;
    rclcpp::Client<SetBool>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;

    void send_deactivate()
    {
        if (!client_->service_is_ready())
        {
            RCLCPP_INFO(this->get_logger(), "Waiting for service to be ready...");
            return;
        }

        auto request = std::make_shared<SetBool::Request>();
        request->data = false;
        client_->async_send_request(request, [this](rclcpp::Client<SetBool>::SharedFuture future)
                                    {
            const auto response = future.get();
            if (!response->success)
            {
                RCLCPP_WARN(get_logger(), "Request failed: %s", response->message.c_str());
                return;
            }
            RCLCPP_INFO(get_logger(), "Request succeeded: %s", response->message.c_str()); });
        timer_->cancel();
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DetectionSupervisorNode>();
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node->get_node_base_interface());
    executor.spin();
    rclcpp::shutdown();
    return 0;
}