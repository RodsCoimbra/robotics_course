#include <string_view>
#include <optional>
#include <iostream>
#include <string>

enum class LocalizationState
{
    Uninitialized,
    Localizing,
    Localized,
    Lost,
    Error
};

std::string_view toString(LocalizationState state)
{
    switch (state)
    {
    case LocalizationState::Uninitialized:
        return "uninitialized";

    case LocalizationState::Localizing:
        return "localizing";

    case LocalizationState::Localized:
        return "localized";

    case LocalizationState::Lost:
        return "lost";

    case LocalizationState::Error:
        return "error";

    default:
        return "unknown state";
    }
}

class LocalizationMonitor
{
public:
    // 2. Deve alterar o estado atual.
    void setState(LocalizationState state)
    {
        state_ = state;
    }

    // 3. Deve devolver o estado atual.
    // O estado existe sempre, por isso não uses optional nem pointer.
    LocalizationState getState() const
    {
        return state_;
    }

    // 4. Actualiza a precisão estimada da pose, em metros.
    // Para este exercício, valores negativos devem ser ignorados.
    void updatePoseAccuracy(double accuracy)
    {
        if (accuracy < 0.0)
            return;

        accuracy_ = accuracy;
    }

    // 5. Devolve a precisão, se já tiver sido recebida.
    // Pode ainda não existir uma estimativa.
    std::optional<double> getPoseAccuracy() const
    {
        return accuracy_;
    }

    // 6. Deve imprimir algo como:
    // "Localization state: localized | Accuracy: 0.12 m"
    // Se não houver precisão:
    // "Localization state: localizing | Accuracy: unavailable"
    void printStatus() const
    {
        std::cout << "Localization state: "
                  << toString(state_) << " | Accuracy: "
                  << (accuracy_ ? (std::to_string(*accuracy_) + " m") : "unavailable")
                  << std::endl;
    }

private:
    LocalizationState state_{LocalizationState::Uninitialized};
    std::optional<double> accuracy_;
};

int main()
{
    LocalizationMonitor monitor;

    monitor.printStatus();

    monitor.setState(LocalizationState::Localizing);
    monitor.updatePoseAccuracy(-1.0); // deve ser ignorado
    monitor.printStatus();

    monitor.updatePoseAccuracy(0.12);
    monitor.setState(LocalizationState::Localized);
    monitor.printStatus();
    monitor.updatePoseAccuracy(0.12);
    monitor.updatePoseAccuracy(-5.0);

    const auto accuracy = monitor.getPoseAccuracy();

    if (accuracy)
    {
        std::cout << "Stored accuracy: " << *accuracy << " m\n";
    }
}