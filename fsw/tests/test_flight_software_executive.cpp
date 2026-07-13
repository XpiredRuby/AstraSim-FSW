#include "astra/flight_software_executive.hpp"

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

astra::FlightSoftwareExecutiveInput input_for_tick(std::uint64_t tick) {
    astra::FlightSoftwareExecutiveInput input;
    input.tick = tick;
    input.flight_input.timestamp_ms = 1000U + tick * 100U;
    input.flight_input.cpu_load_percent = 10.0F;
    input.flight_input.memory_load_percent = 20.0F;
    input.flight_input.heartbeat_count = static_cast<std::uint32_t>(tick);
    input.flight_input.sensor_age_ms = 0U;
    input.flight_input.payload_age_ms = 0U;
    input.flight_input.loop_duration_ms = 10U;
    return input;
}

void test_default_executive_is_valid() {
    astra::FlightSoftwareExecutive executive;
    expect_true(executive.valid(), "default executive configuration is valid");
    expect_true(executive.validation_error().empty(), "valid executive has no error");
}

void test_invalid_executive_configuration_is_rejected() {
    astra::FlightSoftwareExecutiveConfig configuration;
    configuration.flight_period_ticks = 0U;
    astra::FlightSoftwareExecutive executive(configuration);

    expect_true(!executive.valid(), "zero flight period invalidates executive");
    const auto output = executive.step(input_for_tick(0U));
    expect_true(
        output.scheduler_status == astra::SchedulerStatus::INVALID_CONFIGURATION,
        "invalid executive refuses execution"
    );
    expect_true(!output.flight_software_ran, "invalid executive does not run flight software");
}

void test_default_release_pattern() {
    astra::FlightSoftwareExecutive executive;

    const auto tick0 = executive.step(input_for_tick(0U));
    const auto tick1 = executive.step(input_for_tick(1U));
    const auto tick9 = [&executive]() {
        astra::FlightSoftwareExecutiveOutput output;
        for (std::uint64_t tick = 2U; tick <= 9U; ++tick) {
            output = executive.step(input_for_tick(tick));
        }
        return output;
    }();
    const auto tick10 = executive.step(input_for_tick(10U));

    expect_true(tick0.flight_software_ran, "flight software releases at tick zero");
    expect_true(tick0.housekeeping_ran, "housekeeping releases at tick zero");
    expect_true(tick1.flight_software_ran, "flight software releases every tick");
    expect_true(!tick1.housekeeping_ran, "housekeeping does not release at tick one");
    expect_true(!tick9.housekeeping_ran, "housekeeping remains inactive through tick nine");
    expect_true(tick10.housekeeping_ran, "housekeeping releases again at tick ten");
    expect_true(
        tick10.flight_output.telemetry.sequence_number == 11U,
        "scheduled app executes exactly once per base tick"
    );
}

void test_scheduled_command_changes_mode() {
    astra::FlightSoftwareExecutive executive;
    auto input = input_for_tick(0U);
    input.flight_input.has_command = true;
    input.flight_input.command.sequence_number = 1U;
    input.flight_input.command.timestamp_ms = input.flight_input.timestamp_ms;
    input.flight_input.command.command_id = astra::CommandId::SET_MODE;
    input.flight_input.command.argument = static_cast<std::uint32_t>(astra::Mode::NOMINAL);

    const auto output = executive.step(input);

    expect_true(output.flight_software_ran, "scheduled flight step ran");
    expect_true(
        output.flight_output.command_result.status == astra::CommandStatus::ACCEPTED,
        "scheduled SET_MODE command accepted"
    );
    expect_true(executive.current_mode() == astra::Mode::NOMINAL, "scheduled app reaches NOMINAL");
    expect_true(
        output.flight_completion.status == astra::CompletionStatus::COMPLETED_ON_TIME,
        "scheduled flight step completes on time"
    );
}

void test_simulated_execution_delay_records_late_completion() {
    astra::FlightSoftwareExecutive executive;
    auto input = input_for_tick(0U);
    input.simulated_flight_completion_delay_ticks = 2U;

    const auto output = executive.step(input);

    expect_true(
        output.flight_completion.status == astra::CompletionStatus::COMPLETED_LATE,
        "completion beyond deadline is late"
    );
    expect_true(
        output.scheduler_statistics.at(0).deadline_miss_count == 1U,
        "late scheduled completion increments miss count"
    );
}

void test_tick_discontinuity_prevents_application_execution() {
    astra::FlightSoftwareExecutive executive;
    static_cast<void>(executive.step(input_for_tick(0U)));
    const auto skipped = executive.step(input_for_tick(2U));
    const auto recovered = executive.step(input_for_tick(1U));

    expect_true(
        skipped.scheduler_status == astra::SchedulerStatus::TICK_DISCONTINUITY,
        "executive rejects skipped tick"
    );
    expect_true(!skipped.flight_software_ran, "skipped tick cannot execute app");
    expect_true(recovered.scheduler_status == astra::SchedulerStatus::OK, "expected tick remains recoverable");
    expect_true(recovered.flight_software_ran, "recovered tick executes app");
}

}  // namespace

int main() {
    std::cout << "Running FlightSoftwareExecutive tests..." << std::endl;

    test_default_executive_is_valid();
    test_invalid_executive_configuration_is_rejected();
    test_default_release_pattern();
    test_scheduled_command_changes_mode();
    test_simulated_execution_delay_records_late_completion();
    test_tick_discontinuity_prevents_application_execution();

    if (failures == 0) {
        std::cout << "All FlightSoftwareExecutive tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " FlightSoftwareExecutive test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
