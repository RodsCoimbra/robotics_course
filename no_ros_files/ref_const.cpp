#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

struct RobotStatus
{
    std::string robot_id;
    double battery_percent;
    bool emergency_stop;
};

class FleetMonitor
{
public:
    // 1. Deve apenas imprimir o estado recebido, sem o alterar.
    void logStatus(const RobotStatus &robot_status) const
    {
        std::cout << "Robot: " << robot_status.robot_id
                  << " | Battery: " << robot_status.battery_percent
                  << "% | Emergency stop: "
                  << (robot_status.emergency_stop ? "ON" : "OFF")
                  << '\n';
    }

    // 2. Deve limitar battery_percent ao intervalo [0.0, 100.0],
    //    alterando o objeto entregue pelo chamador.
    void clampBattery(RobotStatus &robot_status)
    {
        robot_status.battery_percent = std::clamp(
            robot_status.battery_percent,
            0.0, 100.0);
    }

    // 3. Deve guardar internamente uma cópia própria do frame id.
    //    Mais tarde vamos usar std::move na implementação.
    void setGlobalFrame(std::string frame)
    {
        global_frame_ = std::move(frame);
    }

    // 4. Deve devolver o estado do robô pelo id.
    //    O robô pode não existir; esta versão não permite alterações.
    const RobotStatus *findRobot(const std::string &id) const
    {
        const auto it = std::find_if(robots_.begin(), robots_.end(), [&id](const RobotStatus &robot_status)
                                     { return robot_status.robot_id == id; });

        if (robots_.end() == it)
        {
            return nullptr;
        }

        return &(*it);
    }

    RobotStatus *findRobot(const std::string &id)
    {
        const auto it = std::find_if(robots_.begin(), robots_.end(), [&id](const RobotStatus &robot_status)
                                     { return robot_status.robot_id == id; });

        if (robots_.end() == it)
        {
            return nullptr;
        }

        return &(*it);
    }

private:
    std::string global_frame_;
    std::vector<RobotStatus> robots_;
};

int main(int argc, char const *argv[])
{

    return 0;
}
