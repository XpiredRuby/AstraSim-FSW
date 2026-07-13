#include "astra/command_packet.hpp"
#include "astra/command_processor.hpp"
#include "astra/flight_software_app.hpp"
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

std::uint64_t unix_time_ms() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
    );
}

void print_usage(const char* executable_name) {
    std::cout
        << "Usage: " << executable_name
        << " [command_port] [telemetry_ip] [telemetry_port] [loop_count] [sensor_timeout_loop] [watchdog_timeout_loop]\n"
        << "\n"
        << "Example:\n"
        << "  " << executable_name << " 6000 127.0.0.1 5005 30\n"
        << "  " << executable_name << " 6000 127.0.0.1 5005 30 8\n"
        << "  " << executable_name << " 6000 127.0.0.1 5005 30 0 8\n";
}

float cpu_load_for_state(astra::FaultCode fault) {
    if (fault == astra::FaultCode::CPU_OVERLOAD) {
        return 92.0F;
    }

    return 18.0F;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::uint16_t command_port = 6000;
    std::string telemetry_ip = "127.0.0.1";
    std::uint16_t telemetry_port = 5005;
    std::uint32_t loop_count = 30;
    std::uint32_t sensor_timeout_loop = 0;
    std::uint32_t watchdog_timeout_loop = 0;

    if (argc == 2) {
        const std::string arg = argv[1];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        command_port = static_cast<std::uint16_t>(std::stoi(arg));
    } else if (argc == 5 || argc == 6 || argc == 7) {
        command_port = static_cast<std::uint16_t>(std::stoi(argv[1]));
        telemetry_ip = argv[2];
        telemetry_port = static_cast<std::uint16_t>(std::stoi(argv[3]));
        loop_count = static_cast<std::uint32_t>(std::stoul(argv[4]));

        if (argc >= 6) {
            sensor_timeout_loop = static_cast<std::uint32_t>(std::stoul(argv[5]));
        }

        if (argc == 7) {
            watchdog_timeout_loop = static_cast<std::uint32_t>(std::stoul(argv[6]));
        }
    } else if (argc != 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    astra::FlightSoftwareApp app;

    astra::UdpCommandReceiver command_receiver(command_port, "0.0.0.0", 50);
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
    std::cout << "Using FlightSoftwareApp core loop" << std::endl;
    std::cout << "Listening for UDP commands on port " << command_receiver.bound_port() << std::endl;
    std::cout << "Sending UDP telemetry to " << telemetry_ip << ":" << telemetry_port << std::endl;

    for (std::uint32_t loop = 1; loop <= loop_count; ++loop) {
        if (watchdog_timeout_loop != 0U && loop == watchdog_timeout_loop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(650));
        }

        astra::FlightSoftwareStepInput input;
        input.timestamp_ms = unix_time_ms();
        input.cpu_load_percent = cpu_load_for_state(app.last_fault());
        input.memory_load_percent = 42.0F + static_cast<float>(loop) * 0.25F;
        input.heartbeat_count = loop;

        if (sensor_timeout_loop != 0U && loop >= sensor_timeout_loop) {
            input.sensor_age_ms = 1500;
        }

        astra::CommandPacket command;
        if (command_receiver.receive_packet(command)) {
            input.has_command = true;
            input.command = command;
        }

        const auto output = app.step(input);

        if (output.command_processed) {
            std::cout << "RX command seq=" << output.command_result.sequence_number
                      << " id=" << astra::command_id_to_string(output.command_result.command_id)
                      << " status=" << astra::command_status_to_string(output.command_result.status)
                      << " mode=" << astra::mode_to_string(output.command_result.resulting_mode)
                      << " fault=" << astra::fault_to_string(output.command_result.resulting_fault)
                      << " msg=\"" << output.command_result.message << "\""
                      << std::endl;
        }

        if (output.watchdog_fault_processed) {
            std::cout << "AUTO watchdog fault status="
                      << astra::command_status_to_string(output.watchdog_fault_result.status)
                      << " mode=" << astra::mode_to_string(output.watchdog_fault_result.resulting_mode)
                      << " fault=" << astra::fault_to_string(output.watchdog_fault_result.resulting_fault)
                      << std::endl;
        }

        if (output.health_fault_processed) {
            std::cout << "AUTO health fault status="
                      << astra::command_status_to_string(output.health_fault_result.status)
                      << " mode=" << astra::mode_to_string(output.health_fault_result.resulting_mode)
                      << " fault=" << astra::fault_to_string(output.health_fault_result.resulting_fault)
                      << std::endl;
        }

        const bool sent = telemetry_sender.send_packet(output.telemetry);

        std::cout << "TX telemetry seq=" << output.telemetry.sequence_number
                  << " mode=" << astra::mode_to_string(output.telemetry.mode)
                  << " fault=" << astra::fault_to_string(output.telemetry.last_fault)
                  << " sent=" << (sent ? "yes" : "no")
                  << std::endl;

        if (!sent) {
            return EXIT_FAILURE;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    std::cout << "Command/telemetry demo complete." << std::endl;
    return EXIT_SUCCESS;
}
