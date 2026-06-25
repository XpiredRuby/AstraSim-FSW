#include "astra/flight_software_app.hpp"

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

astra::CommandPacket make_command(
    astra::CommandId command_id,
    std::uint32_t argument = 0,
    std::uint32_t sequence_number = 1
) {
    astra::CommandPacket packet;
    packet.sequence_number = sequence_number;
    packet.timestamp_ms = 12345;
    packet.command_id = command_id;
    packet.argument = argument;
    return packet;
}

astra::FlightSoftwareStepInput make_input_without_command(std::uint32_t heartbeat) {
    astra::FlightSoftwareStepInput input;
    input.has_command = false;
    input.timestamp_ms = 1000ULL + heartbeat;
    input.cpu_load_percent = 10.0F + static_cast<float>(heartbeat);
    input.memory_load_percent = 40.0F;
    input.heartbeat_count = heartbeat;
    input.sensor_age_ms = 10;
    input.payload_age_ms = 20;
    input.loop_duration_ms = 25;
    return input;
}

astra::FlightSoftwareStepInput make_input_with_command(
    const astra::CommandPacket& command,
    std::uint32_t heartbeat
) {
    astra::FlightSoftwareStepInput input = make_input_without_command(heartbeat);
    input.has_command = true;
    input.command = command;
    return input;
}

void command_app_to_nominal(astra::FlightSoftwareApp& app) {
    app.step(
        make_input_with_command(
            make_command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::NOMINAL),
                1
            ),
            1
        )
    );
}

void test_initial_step_reports_boot() {
    astra::FlightSoftwareApp app;

    const auto output = app.step(make_input_without_command(1));

    expect_true(!output.command_processed, "step without command does not process command");
    expect_true(!output.health_fault_processed, "nominal health does not inject fault");
    expect_true(output.health_report.status == astra::HealthStatus::OK, "initial health reports OK");
    expect_true(output.telemetry.sequence_number == 1, "first telemetry sequence is 1");
    expect_true(output.telemetry.mode == astra::Mode::BOOT, "initial telemetry reports BOOT");
    expect_true(output.telemetry.last_fault == astra::FaultCode::NONE, "initial telemetry fault is NONE");
}

void test_set_mode_command_changes_mode_to_nominal() {
    astra::FlightSoftwareApp app;

    const auto output = app.step(
        make_input_with_command(
            make_command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::NOMINAL),
                1
            ),
            1
        )
    );

    expect_true(output.command_processed, "SET_MODE command processed");
    expect_true(
        output.command_result.status == astra::CommandStatus::ACCEPTED,
        "SET_MODE command accepted"
    );
    expect_true(app.current_mode() == astra::Mode::NOMINAL, "app mode becomes NOMINAL");
    expect_true(output.telemetry.mode == astra::Mode::NOMINAL, "telemetry reports NOMINAL");
}

void test_cpu_fault_command_changes_mode_to_degraded_payload() {
    astra::FlightSoftwareApp app;

    command_app_to_nominal(app);

    const auto output = app.step(
        make_input_with_command(
            make_command(
                astra::CommandId::INJECT_FAULT,
                static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD),
                2
            ),
            2
        )
    );

    expect_true(
        output.command_result.status == astra::CommandStatus::ACCEPTED,
        "INJECT_FAULT command accepted"
    );
    expect_true(
        app.current_mode() == astra::Mode::DEGRADED_PAYLOAD,
        "app mode becomes DEGRADED_PAYLOAD"
    );
    expect_true(app.last_fault() == astra::FaultCode::CPU_OVERLOAD, "app stores CPU_OVERLOAD");
    expect_true(
        output.telemetry.mode == astra::Mode::DEGRADED_PAYLOAD,
        "telemetry reports DEGRADED_PAYLOAD"
    );
    expect_true(
        output.telemetry.last_fault == astra::FaultCode::CPU_OVERLOAD,
        "telemetry reports CPU_OVERLOAD"
    );
}

void test_bad_command_does_not_change_mode() {
    astra::FlightSoftwareApp app;

    const auto output = app.step(
        make_input_with_command(
            make_command(astra::CommandId::SET_MODE, 99, 1),
            1
        )
    );

    expect_true(
        output.command_result.status == astra::CommandStatus::REJECTED_BAD_ARGUMENT,
        "bad command rejected"
    );
    expect_true(app.current_mode() == astra::Mode::BOOT, "bad command keeps app in BOOT");
    expect_true(output.telemetry.mode == astra::Mode::BOOT, "telemetry still reports BOOT");
}

void test_telemetry_sequence_increments_each_step() {
    astra::FlightSoftwareApp app;

    const auto first = app.step(make_input_without_command(1));
    const auto second = app.step(make_input_without_command(2));
    const auto third = app.step(make_input_without_command(3));

    expect_true(first.telemetry.sequence_number == 1, "first sequence is 1");
    expect_true(second.telemetry.sequence_number == 2, "second sequence is 2");
    expect_true(third.telemetry.sequence_number == 3, "third sequence is 3");
}

void test_critical_health_fault_is_injected_automatically() {
    astra::FlightSoftwareApp app;

    command_app_to_nominal(app);

    auto input = make_input_without_command(2);
    input.cpu_load_percent = 97.0F;

    const auto output = app.step(input);

    expect_true(
        output.health_report.status == astra::HealthStatus::CRITICAL,
        "critical CPU load creates critical health report"
    );
    expect_true(
        output.health_report.fault == astra::FaultCode::CPU_OVERLOAD,
        "critical CPU load maps to CPU_OVERLOAD"
    );
    expect_true(output.health_fault_processed, "critical health fault is processed");
    expect_true(
        output.health_fault_result.status == astra::CommandStatus::ACCEPTED,
        "automatic health fault command accepted"
    );
    expect_true(
        app.current_mode() == astra::Mode::DEGRADED_PAYLOAD,
        "automatic CPU fault changes mode to DEGRADED_PAYLOAD"
    );
    expect_true(
        output.telemetry.last_fault == astra::FaultCode::CPU_OVERLOAD,
        "telemetry reports automatic CPU fault"
    );
}

void test_warning_health_does_not_inject_fault() {
    astra::FlightSoftwareApp app;

    auto input = make_input_without_command(1);
    input.cpu_load_percent = 85.0F;

    const auto output = app.step(input);

    expect_true(
        output.health_report.status == astra::HealthStatus::WARNING,
        "warning CPU load creates warning health report"
    );
    expect_true(!output.health_fault_processed, "warning health does not inject fault");
    expect_true(app.current_mode() == astra::Mode::BOOT, "warning health keeps current mode");
    expect_true(output.telemetry.last_fault == astra::FaultCode::NONE, "warning health has no telemetry fault");
}

void test_sensor_timeout_drives_safe_mode() {
    astra::FlightSoftwareApp app;

    command_app_to_nominal(app);

    auto input = make_input_without_command(2);
    input.sensor_age_ms = 1500;

    const auto output = app.step(input);

    expect_true(
        output.health_report.fault == astra::FaultCode::SENSOR_TIMEOUT,
        "sensor timeout maps to SENSOR_TIMEOUT"
    );
    expect_true(output.health_fault_processed, "sensor timeout is processed as health fault");
    expect_true(
        app.current_mode() == astra::Mode::SAFE,
        "sensor timeout changes mode to SAFE"
    );
    expect_true(
        output.telemetry.mode == astra::Mode::SAFE,
        "telemetry reports SAFE"
    );
}

}  // namespace

int main() {
    std::cout << "Running FlightSoftwareApp tests..." << std::endl;

    test_initial_step_reports_boot();
    test_set_mode_command_changes_mode_to_nominal();
    test_cpu_fault_command_changes_mode_to_degraded_payload();
    test_bad_command_does_not_change_mode();
    test_telemetry_sequence_increments_each_step();
    test_critical_health_fault_is_injected_automatically();
    test_warning_health_does_not_inject_fault();
    test_sensor_timeout_drives_safe_mode();

    if (failures == 0) {
        std::cout << "All FlightSoftwareApp tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " FlightSoftwareApp test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
