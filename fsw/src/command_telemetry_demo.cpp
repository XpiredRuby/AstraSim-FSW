#include "astra/command_packet.hpp"
#include "astra/command_processor.hpp"
#include "astra/mode_manager.hpp"
#include "astra/telemetry_packet.hpp"
#include "astra/udp_command_receiver.hpp"
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
    std::cout
        << "Usage: " << executable_name
        << " [command_port] [telemetry_ip] [telemetry_port] [loop_count]\n"
        << "\n"
        << "Example:\n"
        << "  " << executable_name << " 6000 127.0.0.1 5005 30\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::uint16_t command_port = 6000;
    std::string telemetry_ip = "127.0.0.1";
    std::uint16_t telemetry_port = 5005;
    std::uint32_t loop_count = 30;

    if (argc == 2) {
        const std::string arg = argv[1];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        command_port = static_cast<std::uint16_t>(std::stoi(arg));
    } else if (argc == 5) {
        command_port = static_cast<std::uint16_t>(std::stoi(argv[1]));
        telemetry_ip = argv[2];
        telemetry_port = static_cast<std::uint16_t>(std::stoi(argv[3]));
        loop_count = static_cast<std::uint32_t>(std::stoul(argv[4]));
    } else if (argc != 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    astra::ModeManager mode_manager;
    astra::CommandProcessor command_processor(mode_manager);

    astra::UdpCommandReceiver command_receiver(command_port, "0.0.0.0", 100);
    astra::UdpTelemetrySender telemetry_sender(telemetry_ip, telemetry_port);

    if (!command_receiver.is_ready()) {
        std::cerr << "Command receiver failed to initialize." << std::endl;
        return EXIT_FAILURE;
    }

    if (!telemetry_sender.is_ready()) {
        std::cerr << "Telemetry sender failed to initialize." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "AstraSim-FSW command/telemetry demo" << std::endl;
    std::cout << "Listening for UDP commands on port " << command_receiver.bound_port() << std::endl;
    std::cout << "Sending UDP telemetry to " << telemetry_ip << ":" << telemetry_port << std::endl;

    for (std::uint32_t loop = 1; loop <= loop_count; ++loop) {
        astra::CommandPacket command;

        if (command_receiver.receive_packet(command)) {
            const auto result = command_processor.process(command);

            std::cout << "RX command seq=" << result.sequence_number
                      << " id=" << astra::command_id_to_string(result.command_id)
                      << " status=" << astra::command_status_to_string(result.status)
                      << " mode=" << astra::mode_to_string(result.resulting_mode)
                      << " fault=" << astra::fault_to_string(result.resulting_fault)
                      << " msg=\"" << result.message << "\""
                      << std::endl;
        }

        astra::TelemetryPacket telemetry;
        telemetry.sequence_number = loop;
        telemetry.timestamp_ms = monotonic_time_ms();
        telemetry.mode = mode_manager.current_mode();
        telemetry.last_fault = command_processor.last_fault();
        telemetry.cpu_load_percent =
            (telemetry.last_fault == astra::FaultCode::CPU_OVERLOAD) ? 92.0F : 18.0F;
        telemetry.memory_load_percent = 42.0F + static_cast<float>(loop) * 0.25F;
        telemetry.heartbeat_count = loop;

        const bool sent = telemetry_sender.send_packet(telemetry);

        std::cout << "TX telemetry seq=" << telemetry.sequence_number
                  << " mode=" << astra::mode_to_string(telemetry.mode)
                  << " fault=" << astra::fault_to_string(telemetry.last_fault)
                  << " sent=" << (sent ? "yes" : "no")
                  << std::endl;

        if (!sent) {
            return EXIT_FAILURE;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    std::cout << "Command/telemetry demo complete." << std::endl;
    return EXIT_SUCCESS;
}
