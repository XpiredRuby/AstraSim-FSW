#include "astra/command_packet.hpp"
#include "astra/flight_software_app.hpp"
#include "astra/telemetry_packet.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect_true(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        ++failures;
    }
}

astra::CommandPacket set_mode_command(astra::Mode mode, std::uint32_t sequence) {
    astra::CommandPacket packet;
    packet.sequence_number = sequence;
    packet.timestamp_ms = 1000U + sequence;
    packet.command_id = astra::CommandId::SET_MODE;
    packet.argument = static_cast<std::uint32_t>(mode);
    return packet;
}

astra::FlightSoftwareStepInput step_input(
    const astra::CommandPacket& command,
    std::uint64_t timestamp_ms
) {
    astra::FlightSoftwareStepInput input;
    input.has_command = true;
    input.command = command;
    input.timestamp_ms = timestamp_ms;
    input.cpu_load_percent = 10.0F;
    input.memory_load_percent = 20.0F;
    input.heartbeat_count = static_cast<std::uint32_t>(timestamp_ms);
    input.sensor_age_ms = 5U;
    input.payload_age_ms = 5U;
    input.loop_duration_ms = 10U;
    return input;
}

void test_command_arguments_accept_extended_modes() {
    astra::Mode mode = astra::Mode::BOOT;

    const bool standby_ok = astra::command_argument_to_mode(
        static_cast<std::uint32_t>(astra::Mode::STANDBY),
        mode
    );
    expect_true(standby_ok, "STANDBY command argument accepted");
    expect_true(mode == astra::Mode::STANDBY, "STANDBY command argument decoded");

    const bool test_ok = astra::command_argument_to_mode(
        static_cast<std::uint32_t>(astra::Mode::TEST),
        mode
    );
    expect_true(test_ok, "TEST command argument accepted");
    expect_true(mode == astra::Mode::TEST, "TEST command argument decoded");
}

void test_telemetry_round_trips_extended_modes() {
    for (const auto mode : {astra::Mode::STANDBY, astra::Mode::TEST}) {
        astra::TelemetryPacket packet;
        packet.sequence_number = 42U;
        packet.timestamp_ms = 123456U;
        packet.mode = mode;
        packet.last_fault = astra::FaultCode::NONE;
        packet.cpu_load_percent = 12.5F;
        packet.memory_load_percent = 33.5F;
        packet.heartbeat_count = 7U;

        const auto bytes = astra::serialize_telemetry_packet(packet);
        astra::TelemetryPacket decoded;
        const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

        expect_true(ok, "extended mode telemetry deserializes");
        expect_true(decoded.mode == mode, "extended mode telemetry round trips");
    }
}

void test_flight_software_executes_guarded_test_sequence() {
    astra::FlightSoftwareApp app;

    const auto standby = app.step(step_input(set_mode_command(astra::Mode::STANDBY, 1U), 1001U));
    const auto test = app.step(step_input(set_mode_command(astra::Mode::TEST, 2U), 1002U));
    const auto direct_nominal = app.step(step_input(set_mode_command(astra::Mode::NOMINAL, 3U), 1003U));
    const auto back_to_standby = app.step(step_input(set_mode_command(astra::Mode::STANDBY, 4U), 1004U));
    const auto nominal = app.step(step_input(set_mode_command(astra::Mode::NOMINAL, 5U), 1005U));

    expect_true(
        standby.command_result.status == astra::CommandStatus::ACCEPTED,
        "BOOT -> STANDBY command accepted"
    );
    expect_true(standby.telemetry.mode == astra::Mode::STANDBY, "telemetry reports STANDBY");
    expect_true(test.command_result.status == astra::CommandStatus::ACCEPTED, "STANDBY -> TEST accepted");
    expect_true(test.telemetry.mode == astra::Mode::TEST, "telemetry reports TEST");
    expect_true(
        direct_nominal.command_result.status == astra::CommandStatus::REJECTED_INVALID_TRANSITION,
        "TEST -> NOMINAL direct command rejected"
    );
    expect_true(direct_nominal.telemetry.mode == astra::Mode::TEST, "rejected TEST exit preserves mode");
    expect_true(
        back_to_standby.command_result.status == astra::CommandStatus::ACCEPTED,
        "TEST -> STANDBY command accepted"
    );
    expect_true(nominal.command_result.status == astra::CommandStatus::ACCEPTED, "STANDBY -> NOMINAL accepted");
    expect_true(nominal.telemetry.mode == astra::Mode::NOMINAL, "telemetry reports final NOMINAL");
}

}  // namespace

int main() {
    std::cout << "Running extended mode tests..." << std::endl;

    test_command_arguments_accept_extended_modes();
    test_telemetry_round_trips_extended_modes();
    test_flight_software_executes_guarded_test_sequence();

    if (failures == 0) {
        std::cout << "All extended mode tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " extended mode test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
