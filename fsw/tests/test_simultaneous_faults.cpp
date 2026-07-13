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

astra::FlightSoftwareStepInput nominal_input(std::uint64_t timestamp_ms) {
    astra::FlightSoftwareStepInput input;
    input.timestamp_ms = timestamp_ms;
    input.cpu_load_percent = 10.0F;
    input.memory_load_percent = 20.0F;
    input.heartbeat_count = static_cast<std::uint32_t>(timestamp_ms);
    input.sensor_age_ms = 0U;
    input.payload_age_ms = 0U;
    input.loop_duration_ms = 20U;
    return input;
}

astra::CommandPacket command(
    astra::CommandId command_id,
    std::uint32_t argument,
    std::uint32_t sequence
) {
    astra::CommandPacket packet;
    packet.sequence_number = sequence;
    packet.timestamp_ms = 1000U + sequence;
    packet.command_id = command_id;
    packet.argument = argument;
    return packet;
}

void initialize_nominal(astra::FlightSoftwareApp& app) {
    auto input = nominal_input(1000U);
    input.has_command = true;
    input.command = command(
        astra::CommandId::SET_MODE,
        static_cast<std::uint32_t>(astra::Mode::NOMINAL),
        1U
    );
    static_cast<void>(app.step(input));
}

void test_watchdog_wins_over_cpu_fault() {
    astra::FlightSoftwareApp app;
    initialize_nominal(app);

    auto input = nominal_input(1700U);
    input.cpu_load_percent = 97.0F;
    const auto output = app.step(input);

    expect_true(
        output.watchdog_report.status == astra::WatchdogStatus::EXPIRED,
        "simultaneous case contains expired watchdog"
    );
    expect_true(
        output.health_report.status == astra::HealthStatus::CRITICAL &&
            output.health_report.fault == astra::FaultCode::CPU_OVERLOAD,
        "simultaneous case contains critical CPU fault"
    );
    expect_true(output.internal_fault_processed, "one primary internal fault is processed");
    expect_true(
        output.primary_internal_fault == astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        "watchdog is selected as primary fault"
    );
    expect_true(output.watchdog_fault_processed, "watchdog source records processed fault");
    expect_true(!output.health_fault_processed, "lower-priority CPU fault is not processed in same cycle");
    expect_true(app.current_mode() == astra::Mode::SAFE, "primary watchdog fault enters SAFE");
    expect_true(
        app.last_fault() == astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        "telemetry fault identity remains primary watchdog fault"
    );
}

void test_active_fault_reasserts_after_clear_command() {
    astra::FlightSoftwareApp app;
    initialize_nominal(app);

    auto input = nominal_input(1100U);
    input.cpu_load_percent = 97.0F;
    input.has_command = true;
    input.command = command(astra::CommandId::CLEAR_FAULT, 0U, 2U);
    const auto output = app.step(input);

    expect_true(
        output.command_result.status == astra::CommandStatus::ACCEPTED,
        "CLEAR_FAULT command is accepted"
    );
    expect_true(
        output.primary_internal_fault == astra::FaultCode::CPU_OVERLOAD,
        "still-active CPU fault is selected after clear"
    );
    expect_true(output.health_fault_processed, "active CPU condition is reprocessed");
    expect_true(
        app.last_fault() == astra::FaultCode::CPU_OVERLOAD,
        "active CPU fault is reasserted after clear"
    );
    expect_true(
        app.current_mode() == astra::Mode::DEGRADED_PAYLOAD,
        "active CPU fault keeps degraded payload response"
    );
}

}  // namespace

int main() {
    std::cout << "Running simultaneous fault tests..." << std::endl;

    test_watchdog_wins_over_cpu_fault();
    test_active_fault_reasserts_after_clear_command();

    if (failures == 0) {
        std::cout << "All simultaneous fault tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " simultaneous fault test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
