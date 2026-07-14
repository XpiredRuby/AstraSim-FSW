#include "astra/flight_software_app.hpp"
#include "astra/recovery_supervisor.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect_true(bool condition, const std::string& name) {
    if (condition) {
        std::cout << "[PASS] " << name << std::endl;
    } else {
        std::cout << "[FAIL] " << name << std::endl;
        ++failures;
    }
}

astra::CommandPacket command(
    astra::CommandId id,
    std::uint32_t argument,
    std::uint32_t sequence
) {
    astra::CommandPacket packet;
    packet.command_id = id;
    packet.argument = argument;
    packet.sequence_number = sequence;
    packet.timestamp_ms = 1000U + sequence;
    return packet;
}

astra::FlightSoftwareStepInput input_for(const astra::CommandPacket& packet) {
    astra::FlightSoftwareStepInput input;
    input.has_command = true;
    input.command = packet;
    input.timestamp_ms = packet.timestamp_ms;
    input.cpu_load_percent = 10.0F;
    input.memory_load_percent = 20.0F;
    input.sensor_age_ms = 0U;
    input.payload_age_ms = 0U;
    input.loop_duration_ms = 1U;
    return input;
}

void move_to_recovery(astra::ModeManager& manager) {
    static_cast<void>(manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE));
    static_cast<void>(manager.transition_to(astra::Mode::SAFE, astra::FaultCode::SENSOR_TIMEOUT));
    static_cast<void>(manager.transition_to(astra::Mode::RECOVERY, astra::FaultCode::SENSOR_TIMEOUT));
}

void test_invalid_configuration_is_rejected() {
    astra::RecoverySupervisorConfig config;
    config.maximum_failed_attempts = 0U;
    const astra::RecoverySupervisor supervisor(config);

    expect_true(!supervisor.valid(), "zero recovery limit is invalid");
    expect_true(!supervisor.validation_error().empty(), "invalid recovery limit has diagnostic");
}

void test_failure_limit_forces_safe() {
    astra::RecoverySupervisorConfig config;
    config.maximum_failed_attempts = 3U;
    astra::RecoverySupervisor supervisor(config);
    astra::ModeManager manager;
    move_to_recovery(manager);
    supervisor.begin_session();

    const auto first = supervisor.record_failure(manager);
    const auto second = supervisor.record_failure(manager);
    const auto third = supervisor.record_failure(manager);

    expect_true(first.status == astra::RecoveryAttemptStatus::FAILED, "first recovery failure remains bounded");
    expect_true(second.status == astra::RecoveryAttemptStatus::FAILED, "second recovery failure remains bounded");
    expect_true(!first.fail_safe_forced && !second.fail_safe_forced, "SAFE is not forced before limit");
    expect_true(third.status == astra::RecoveryAttemptStatus::FAIL_SAFE_FORCED, "third failure reaches fail-safe limit");
    expect_true(third.failed_attempts == 3U, "failure count is reported");
    expect_true(third.fail_safe_forced, "third failure forces fail-safe");
    expect_true(manager.current_mode() == astra::Mode::SAFE, "fail-safe disposition is SAFE");
}

void test_success_resets_failure_count() {
    astra::RecoverySupervisor supervisor;
    astra::ModeManager manager;
    move_to_recovery(manager);
    supervisor.begin_session();

    static_cast<void>(supervisor.record_failure(manager));
    const auto success = supervisor.record_success();

    expect_true(success.status == astra::RecoveryAttemptStatus::SUCCESS, "successful recovery is recorded");
    expect_true(supervisor.failed_attempts() == 0U, "successful recovery resets failure count");
}

void test_flight_software_forces_safe_after_repeated_failed_exits() {
    astra::FlightSoftwareApp app;

    const auto nominal = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::NOMINAL),
                1U
            )
        )
    );
    const auto fault = app.step(
        input_for(
            command(
                astra::CommandId::INJECT_FAULT,
                static_cast<std::uint32_t>(astra::FaultCode::SENSOR_TIMEOUT),
                2U
            )
        )
    );
    const auto recovery = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::RECOVERY),
                3U
            )
        )
    );
    const auto first = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::TEST),
                4U
            )
        )
    );
    const auto second = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::TEST),
                5U
            )
        )
    );
    const auto third = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::TEST),
                6U
            )
        )
    );

    expect_true(nominal.command_result.status == astra::CommandStatus::ACCEPTED, "NOMINAL setup accepted");
    expect_true(fault.telemetry.mode == astra::Mode::SAFE, "critical fault enters SAFE");
    expect_true(recovery.telemetry.mode == astra::Mode::RECOVERY, "SAFE enters RECOVERY");
    expect_true(first.command_result.status == astra::CommandStatus::REJECTED_INVALID_TRANSITION, "first failed exit rejected");
    expect_true(second.command_result.status == astra::CommandStatus::REJECTED_INVALID_TRANSITION, "second failed exit rejected");
    expect_true(third.command_result.status == astra::CommandStatus::REJECTED_RECOVERY_LIMIT, "third failed exit reports recovery limit");
    expect_true(third.telemetry.mode == astra::Mode::SAFE, "third failed exit forces SAFE telemetry");
    expect_true(
        third.telemetry.last_command_status ==
            static_cast<std::uint8_t>(astra::CommandStatus::REJECTED_RECOVERY_LIMIT),
        "telemetry reports recovery-limit disposition"
    );
}

void test_invalid_recovery_configuration_invalidates_app() {
    astra::FlightSoftwareAppConfig config;
    config.recovery_supervisor.maximum_failed_attempts = 0U;
    const astra::FlightSoftwareApp app(config);

    expect_true(!app.valid(), "invalid recovery configuration invalidates app");
    expect_true(!app.validation_error().empty(), "invalid app exposes recovery diagnostic");
}

void test_status_strings_are_stable() {
    expect_true(
        astra::recovery_attempt_status_to_string(
            astra::RecoveryAttemptStatus::FAIL_SAFE_FORCED
        ) == "FAIL_SAFE_FORCED",
        "recovery status string is stable"
    );
    expect_true(
        astra::command_status_to_string(
            astra::CommandStatus::REJECTED_RECOVERY_LIMIT
        ) == "REJECTED_RECOVERY_LIMIT",
        "command recovery-limit status string is stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running RecoverySupervisor tests..." << std::endl;

    test_invalid_configuration_is_rejected();
    test_failure_limit_forces_safe();
    test_success_resets_failure_count();
    test_flight_software_forces_safe_after_repeated_failed_exits();
    test_invalid_recovery_configuration_invalidates_app();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All RecoverySupervisor tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " RecoverySupervisor test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
