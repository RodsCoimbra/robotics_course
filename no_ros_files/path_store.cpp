#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Waypoint
{
    double x;
    double y;
};

class PathStore
{
public:
    // A classe deve passar a possuir o path internamente.
    // Escolhe a assinatura e implementa o método.
    void setPath(std::vector<Waypoint> path)
    {
        path_ = std::move(path);
    }

    // Devolve apenas leitura; não copies o vector.
    const std::vector<Waypoint> &getPath() const
    {
        return path_;
    }

    void printPathSize() const
    {
        std::cout << "The path size is " << path_.size() << std::endl;
    }

private:
    std::vector<Waypoint> path_;
};

int main()
{
    PathStore store;

    std::vector<Waypoint> planned_path{
        {0.0, 0.0},
        {1.0, 0.5},
        {2.0, 1.0},
    };

    // Caso A: o chamador quer preservar planned_path.
    store.setPath(planned_path);

    std::cout << "Original size after copy: "
              << planned_path.size() << '\n';

    // Caso B: o chamador já não precisa do vector.
    store.setPath(std::move(planned_path));

    std::cout << "Stored path size: "
              << store.getPath().size() << '\n';

    store.printPathSize();

    planned_path = {
        {10.0, 10.0},
        {20.0, 20.0},
        {30.0, 30.0},
        {40.0, 40.0},
    };

    std::cout << "New original size: "
              << planned_path.size() << '\n';

    std::cout << "Stored size remains: "
              << store.getPath().size() << '\n';
}
