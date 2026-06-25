#include "astra/mode_manager.hpp"
#include "astra/telemetry_packet.hpp"
#include "astra/udp_telemetry_sender.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {

std::uint64_t monotonic_time_ms() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
    );
}

void print_usage(const char* executable_name) {
    std::cout << "Usage: " << executable_name << " [destination_ip] [destination_port]\n"
              << "\n"
              << "Example:\n"
              << "  " << executable_name << " 127.0.0.1 5005\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string destination_ip = "127.0.0.1";
    std::uint16_t destination_port = 5005;

    if (argc == 2) {
        const std::string arg = argv[1];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        destination_ip = arg;
    } else if (argc == 3) {
        destination_ip = argv[1];
        destination_port = static_cast<std::uint16_t>(std::stoi(argv[2]));
    } else if (argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    astra::UdpTelemetrySender sender(destination_ip, destination_port);

    if (!sender.is_ready()) {
        std::cerr << "Telemetry sender failed to initialize." << std::endl;
        return EXIT_FAILURE;
    }

    astra::ModeManager mode_manager;

    std::cout << "AstraSim-FSW live telemetry demo" << std::endl;
    std::cout << "Sending UDP telemetry to " << destination_ip << ":" << destination_port << std::endl;

    astra::FaultCode last_fault = astra::FaultCode::NONE;

    for (std::uint32_t heartbeat = 1; heartbeat <= 12; ++heartbeat) {
        if (heartbeat == 2) {
            mode_manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);
        }

        if (heartbeat == 8) {
            last_fault = astra::FaultCode::CPU_OVERLOAD;
            mode_manager.handle_fault(last_fault);
        }

        astra::TelemetryPacket packet;
        packet.sequence_number = heartbeat;
        packet.timestamp_ms = monotonic_time_ms();
        packet.mode = mode_manager.current_mode();
        packet.last_fault = last_fault;
        packet.cpu_load_percent = (heartbeat < 8) ? 18.0F + heartbeat : 91.0F;
        packet.memory_load_percent = 40.0F + static_cast<float>(heartbeat) * 1.5F;
        packet.heartbeat_count = heartbeat;

        const bool sent = sender.send_packet(packet);

        std::cout << "TX seq=" << packet.sequence_number
                  << " mode=" << astra::mode_to_string(packet.mode)
                  << " fault=" << astra::fault_to_string(packet.last_fault)
                  << " sent=" << (sent ? "yes" : "no")
                  << std::endl;

        if (!sent) {
            return EXIT_FAILURE;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Telemetry demo complete." << std::endl;
    return EXIT_SUCCESS;
}
