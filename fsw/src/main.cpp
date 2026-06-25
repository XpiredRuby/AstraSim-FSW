#include "astra/mode_manager.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    astra::ModeManager mode_manager;

    std::cout << "AstraSim-FSW starting..." << std::endl;
    std::cout << "Mode: "
              << astra::mode_to_string(mode_manager.current_mode())
              << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    const auto previous_mode = mode_manager.current_mode();
    const bool transitioned = mode_manager.transition_to(
        astra::Mode::NOMINAL,
        astra::FaultCode::NONE
    );

    if (transitioned) {
        std::cout << "Mode transition: "
                  << astra::mode_to_string(previous_mode)
                  << " -> "
                  << astra::mode_to_string(mode_manager.current_mode())
                  << std::endl;
    } else {
        std::cout << "Mode transition rejected." << std::endl;
    }

    for (int i = 0; i < 5; ++i) {
        std::cout << "Heartbeat " << i + 1
                  << " | Mode: "
                  << astra::mode_to_string(mode_manager.current_mode())
                  << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Injecting simulated CPU overload fault..." << std::endl;

    const auto before_fault = mode_manager.current_mode();
    mode_manager.handle_fault(astra::FaultCode::CPU_OVERLOAD);

    std::cout << "Mode transition: "
              << astra::mode_to_string(before_fault)
              << " -> "
              << astra::mode_to_string(mode_manager.current_mode())
              << " | Reason: "
              << astra::fault_to_string(astra::FaultCode::CPU_OVERLOAD)
              << std::endl;

    std::cout << "AstraSim-FSW shutdown." << std::endl;
    return 0;
}
