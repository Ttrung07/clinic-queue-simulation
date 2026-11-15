#include "SimulationConfig.h"
#include <limits>

namespace clinic {
bool validateSimulationConfig(const SimulationConfig& config, std::string& error) {
    error.clear();
    if (config.closingTime <= 0 || config.closingTime == std::numeric_limits<int>::max()) {
        error = "Gio dong phai trong [1, INT_MAX-1].";
        return false;
    }
    int total = 0;
    for (int count : config.roomsPerSpecialty) {
        if (count <= 0 || count > std::numeric_limits<int>::max() - total) {
            error = "Moi khoa can phong duong; tong so phong khong duoc tran int.";
            return false;
        }
        total += count;
    }
    return true;
}

bool validateGeneratorConfig(const GeneratorConfig& config, std::string& error) {
    error.clear();
    if (config.patientCount < 0)
        error = "So benh nhan khong duoc am.";
    else if (config.closingTime <= 0 || config.closingTime == std::numeric_limits<int>::max())
        error = "Gio dong phai trong [1, INT_MAX-1].";
    else if (config.minServiceDuration <= 0 || config.maxServiceDuration < config.minServiceDuration)
        error = "Khoang thoi luong khong hop le.";
    else if (config.patientCount != 0 &&
             config.maxServiceDuration >
                 (std::numeric_limits<int>::max() - config.closingTime) / config.patientCount)
        error = "Tong thoi luong sinh toi da co the lam tran dong ho.";
    return error.empty();
}
} // namespace clinic
