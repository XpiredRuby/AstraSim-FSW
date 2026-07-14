#include "astra/command_authorizer.hpp"
#include "astra/flight_software_app.hpp"

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
    std::uint32_t argument = 0U,
    std::uint32_t sequence = 1U
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

void test_default_policy_preserves_existing_commands() {
    const astra::CommandAuthorizer authorizer;

    for (const auto id : {
             astra::CommandId::NOOP,
             astra::CommandId::SET_MODE,
             astra::CommandId::INJECT_FAULT,
             astra::CommandId::REQUEST_TELEMETRY,
             astra::CommandId::CLEAR_FAULT,
         }) {
        auto packet = command(id);
        if (id == astra::CommandId::SET_MODE) {
            packet.argument = static_cast<std::uint32_t>(astra::Mode::TEST);
        }
        const auto result = authorizer.authorize(packet);
        expect_true(result.authorized, "default policy authorizes existing command");
        expect_true(
            result.status == astra::CommandAuthorizationStatus::AUTHORIZED,
            "default policy returns AUTHORIZED"
        );
    }
}

void test_policy_rejects_fault_injection_and_test_mode() {
    astra::CommandAuthorizationConfig config;
    config.allow_fault_injection = false;
    config.allow_test_mode = false;
    const astra::CommandAuthorizer authorizer(config);

    const auto fault = authorizer.authorize(
        command(
            astra::CommandId::INJECT_FAULT,
            static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD)
        )
    );
    const auto test_mode = authorizer.authorize(
        command(
            astra::CommandId::SET_MODE,
            static_cast<std::uint32_t>(astra::Mode::TEST)
        )
    );
    const auto invalid_mode = authorizer.authorize(
        command(astra::CommandId::SET_MODE, 999U)
    );

    expect_true(!fault.authorized, "fault injection can be disabled independently");
    expect_true(
        fault.status == astra::CommandAuthorizationStatus::REJECTED_COMMAND,
        "disabled fault injection returns command rejection"
    );
    expect_true(!test_mode.authorized, "TEST mode can be disabled independently");
    expect_true(
        test_mode.status == astra::CommandAuthorizationStatus::REJECTED_ARGUMENT,
        "disabled TEST mode returns argument rejection"
    );
    expect_true(
        invalid_mode.authorized,
        "semantic argument validation remains the command processor responsibility"
    );
}

void test_app_boundary_rejects_unauthorized_command_without_state_change() {
    astra::FlightSoftwareAppConfig config;
    config.command_authorization.allow_fault_injection = false;
    astra::FlightSoftwareApp app(config);

    const auto nominal = app.step(
        input_for(
            command(
                astra::CommandId::SET_MODE,
                static_cast<std::uint32_t>(astra::Mode::NOMINAL),
                1U
            )
        )
    );
    const auto rejected = app.step(
        input_for(
            command(
                astra::CommandId::INJECT_FAULT,
                static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD),
                2U
            )
        )
    );
    const auto duplicate = app.step(
        input_for(command(astra::CommandId::NOOP, 0U, 2U))
    );

    expect_true(
        nominal.command_result.status == astra::CommandStatus::ACCEPTED,
        "authorized mode command is accepted"
    );
    expect_true(
        rejected.command_result.status == astra::CommandStatus::REJECTED_UNAUTHORIZED,
        "unauthorized command has typed flight-software rejection"
    );
    expect_true(
        !rejected.command_authorization_result.authorized,
        "flight-software output exposes authorization disposition"
    );
    expect_true(app.current_mode() == astra::Mode::NOMINAL, "unauthorized command preserves mode");
    expect_true(app.last_fault() == astra::FaultCode::NONE, "unauthorized command preserves fault state");
    expect_true(
        rejected.telemetry.last_command_status ==
            static_cast<std::uint8_t>(astra::CommandStatus::REJECTED_UNAUTHORIZED),
        "telemetry reports authorization rejection"
    );
    expect_true(
        duplicate.command_result.status == astra::CommandStatus::REJECTED_DUPLICATE_SEQUENCE,
        "authorization-rejected command still consumes its accepted sequence"
    );
}

void test_status_strings_are_stable() {
    expect_true(
        astra::command_authorization_status_to_string(
            astra::CommandAuthorizationStatus::AUTHORIZED
        ) == "AUTHORIZED",
        "authorization status string is stable"
    );
    expect_true(
        astra::command_status_to_string(
            astra::CommandStatus::REJECTED_UNAUTHORIZED
        ) == "REJECTED_UNAUTHORIZED",
        "command status string is stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running CommandAuthorizer tests..." << std::endl;

    test_default_policy_preserves_existing_commands();
    test_policy_rejects_fault_injection_and_test_mode();
    test_app_boundary_rejects_unauthorized_command_without_state_change();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All CommandAuthorizer tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " CommandAuthorizer test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
