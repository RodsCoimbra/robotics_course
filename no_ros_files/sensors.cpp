#include <string>
#include <vector>
#include <memory>
#include <iostream>
class Sensor
{
public:
    std::string name;

    Sensor(std::string _name) : name(_name) {};
};

class SensorManager
{
public:
    std::vector<std::unique_ptr<Sensor>> sensors;

    void add_sensor(std::unique_ptr<Sensor> &sensor)
    {
        sensors.push_back(std::move(sensor));
    }

    void show_sensors() const
    {
        for (const auto &a : sensors)
        {
            std::cout << "Sensor: " << a->name << std::endl;
        }
    }

    void removeSensor(size_t index)
    {
        sensors.erase(sensors.begin() + index);
    }

    const Sensor *getSensor(size_t index) const
    {
        if (index > sensors.size())
            return nullptr;

        return sensors[index].get();
    }

    Sensor *getSensor(size_t index)
    {
        if (index > sensors.size())
            return nullptr;

        return sensors[index].get();
    }
};

int main(int argc, char const *argv[])
{
    auto sensor_1 = std::make_unique<Sensor>("LiDAR");
    SensorManager sensors_man;
    sensors_man.add_sensor(sensor_1);
    auto sensor_2 = std::make_unique<Sensor>("Camera");
    sensors_man.add_sensor(sensor_2);
    sensors_man.show_sensors();

    std::cout << "Finalized adding" << std::endl;
    sensors_man.removeSensor(0);
    sensors_man.show_sensors();
    return 0;
}
