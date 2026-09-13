#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
class Sensor
{
public:
    std::string name;
    std::string type; // "LiDAR", "Camera", "IMU", etc.
    bool is_active;

    // Constructor: recebe name e type
    Sensor(std::string _name, std::string _type) : name(_name), type(_type), is_active(false) {}

    // Ativar/desativar sensor
    void activate()
    {
        is_active = true;
    }

    void deactivate()
    {
        is_active = false;
    }
};

class SensorManager
{
private:
    std::vector<std::unique_ptr<Sensor>> sensors;

public:
    // Adicionar sensor (move semantics)
    void add_sensor(std::unique_ptr<Sensor> sensor)
    {
        sensors.push_back(std::move(sensor));
    }

    // Obter sensor por índice (retorna nullptr se inválido)
    Sensor *getSensor(size_t index)
    {
        if (index >= sensors.size())
            return nullptr;

        return sensors[index].get();
    }
    const Sensor *getSensor(size_t index) const
    {
        if (index >= sensors.size())
            return nullptr;

        return sensors[index].get();
    }

    // Obter sensor por nome (retorna nullptr se não existir)
    Sensor *getSensorByName(const std::string &name)
    {
        const auto it = std::find_if(sensors.begin(), sensors.end(), [&name](const auto &sensor)
                                     { return name == sensor->name; });

        if (sensors.end() == it)
            return nullptr;

        return it->get();
    }

    // Remover sensor por índice
    void removeSensor(size_t index)
    {
        if (index < sensors.size())
            sensors.erase(sensors.begin() + index);
    }

    // Ativar todos os sensores de um tipo
    void activateByType(const std::string &type)
    {
        for (auto &sensor : sensors)
        {
            if (type == sensor->type)
                sensor->activate();
        }
    }
    // Contar sensores ativos
    size_t countActive() const
    {
        return std::count_if(sensors.begin(), sensors.end(),
                             [](const auto &sensor)
                             { return sensor->is_active; });
    }

    // Listar todos os sensores (formato: "name [type] - active/inactive")
    void listSensors() const
    {
        for (const auto &sensor : sensors)
        {
            std::cout << sensor->name << " [" << sensor->type << "] - " << (sensor->is_active ? "active" : "inactive") << std::endl;
        }
    }

    // Quantos sensores existem?
    size_t count() const
    {
        return sensors.size();
    }
};

int main()
{
    SensorManager manager;

    // 1. Adiciona 3 sensores
    manager.add_sensor(std::make_unique<Sensor>("LiDAR_Front", "LiDAR"));
    manager.add_sensor(std::make_unique<Sensor>("Camera_Left", "Camera"));
    manager.add_sensor(std::make_unique<Sensor>("IMU_Center", "IMU"));

    // 2. Lista sensores (devem estar todos inativos)
    manager.listSensors();

    // 3. Ativa sensores do tipo "LiDAR"
    manager.activateByType("LiDAR");

    // 4. Obtém sensor por nome e ativa
    Sensor *cam = manager.getSensorByName("Camera_Left");
    if (cam)
        cam->activate();

    // 5. Conta ativos (devem ser 2)
    std::cout << "Active sensors: " << manager.countActive() << "\n";

    // 6. Remove primeiro sensor
    manager.removeSensor(0);

    // 7. Tenta obter sensor removido (deve retornar nullptr)
    Sensor *removed = manager.getSensor(0); // Agora é o antigo sensor 1
    if (removed)
        std::cout << "Found: " << removed->name << "\n";
    else
        std::cout << "Sensor 0 não existe\n";

    // 8. Lista sensores finais
    manager.listSensors();

    return 0;
}