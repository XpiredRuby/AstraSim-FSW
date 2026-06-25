#include <chrono>
#include <iostream>
#include <string>
#include <thread>

enum class Mode {
    BOOT,
    NOMINAL,
    DEGRADED_SENSOR,
    DEGRADED_PAYLOAD,
    SAFE,
    RECOVERY
};

std::string mode_to_string(Mode mode) {
    switch (mode) {
        case Mode::BOOT: return "BOOT";
        case Mode::NOMINAL: return "NOMINAL";
        case Mode::DEGRADED_SENSOR: return "DEGRADED_SENSOR";
        case Mode::DEGRADED_PAYLOAD: return "DEGRADED_PAYLOAD";
        case Mode::SAFE: return "SAFE";
        case Mode::RECOVERY: return "RECOVERY";
        default: return "UNKNOWN";
    }
}

int main() {
    Mode current_mode = Mode::BOOT;

    std::cout << "AstraSim-FSW starting..." << std::endl;
    std::cout << "Mode: " << mode_to_string(current_mode) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    Mode previous_mode = current_mode;
    current_mode = Mode::NOMINAL;

    std::cout << "Mode transition: "
              << mode_to_string(previous_mode)
              << " -> "
              << mode_to_string(current_mode)
              << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::cout << "Heartbeat " << i + 1
                  << " | Mode: "
                  << mode_to_string(current_mode)
                  << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "AstraSim-FSW shutdown." << std::endl;
    return 0;
}
