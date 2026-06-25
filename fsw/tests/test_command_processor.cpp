#include "astra/command_processor.hpp"

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

void test_noop_is_accepted() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    const auto result = processor.process(make_command(astra::CommandId::NOOP));

    expect_true(result.status == astra::CommandStatus::ACCEPTED, "NOOP accepted");
    expect_true(mode_manager.current_mode() == astra::Mode::BOOT, "NOOP does not change mode");
}

void test_set_mode_boot_to_nominal() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    const auto result = processor.process(
        make_command(
            astra::CommandId::SET_MODE,
            static_cast<std::uint32_t>(astra::Mode::NOMINAL)
        )
    );

    expect_true(result.status == astra::CommandStatus::ACCEPTED, "SET_MODE BOOT -> NOMINAL accepted");
    expect_true(mode_manager.current_mode() == astra::Mode::NOMINAL, "mode becomes NOMINAL");
}

void test_set_mode_invalid_transition_rejected() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    const auto result = processor.process(
        make_command(
            astra::CommandId::SET_MODE,
            static_cast<std::uint32_t>(astra::Mode::DEGRADED_PAYLOAD)
        )
    );

    expect_true(
        result.status == astra::CommandStatus::REJECTED_INVALID_TRANSITION,
        "invalid SET_MODE transition rejected"
    );
    expect_true(mode_manager.current_mode() == astra::Mode::BOOT, "mode stays BOOT");
}

void test_set_mode_bad_argument_rejected() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    const auto result = processor.process(make_command(astra::CommandId::SET_MODE, 99));

    expect_true(
        result.status == astra::CommandStatus::REJECTED_BAD_ARGUMENT,
        "SET_MODE bad argument rejected"
    );
    expect_true(mode_manager.current_mode() == astra::Mode::BOOT, "bad SET_MODE does not change mode");
}

void test_inject_fault_cpu_overload() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    processor.process(
        make_command(
            astra::CommandId::SET_MODE,
            static_cast<std::uint32_t>(astra::Mode::NOMINAL)
        )
    );

    const auto result = processor.process(
        make_command(
            astra::CommandId::INJECT_FAULT,
            static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD),
            2
        )
    );

    expect_true(result.status == astra::CommandStatus::ACCEPTED, "INJECT_FAULT accepted");
    expect_true(
        mode_manager.current_mode() == astra::Mode::DEGRADED_PAYLOAD,
        "CPU_OVERLOAD changes mode to DEGRADED_PAYLOAD"
    );
    expect_true(processor.last_fault() == astra::FaultCode::CPU_OVERLOAD, "last fault stored");
}

void test_inject_fault_bad_argument_rejected() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    const auto result = processor.process(make_command(astra::CommandId::INJECT_FAULT, 9999));

    expect_true(
        result.status == astra::CommandStatus::REJECTED_BAD_ARGUMENT,
        "INJECT_FAULT bad argument rejected"
    );
    expect_true(processor.last_fault() == astra::FaultCode::NONE, "bad fault argument does not store fault");
}

void test_clear_fault() {
    astra::ModeManager mode_manager;
    astra::CommandProcessor processor(mode_manager);

    processor.process(
        make_command(
            astra::CommandId::SET_MODE,
            static_cast<std::uint32_t>(astra::Mode::NOMINAL)
        )
    );

    processor.process(
        make_command(
            astra::CommandId::INJECT_FAULT,
            static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD),
            2
        )
    );

    const auto result = processor.process(make_command(astra::CommandId::CLEAR_FAULT, 0, 3));

    expect_true(result.status == astra::CommandStatus::ACCEPTED, "CLEAR_FAULT accepted");
    expect_true(processor.last_fault() == astra::FaultCode::NONE, "fault cleared");
}

void test_status_strings() {
    expect_true(
        astra::command_status_to_string(astra::CommandStatus::REJECTED_BAD_ARGUMENT) ==
            "REJECTED_BAD_ARGUMENT",
        "command status string conversion works"
    );
}

}  // namespace

int main() {
    std::cout << "Running CommandProcessor tests..." << std::endl;

    test_noop_is_accepted();
    test_set_mode_boot_to_nominal();
    test_set_mode_invalid_transition_rejected();
    test_set_mode_bad_argument_rejected();
    test_inject_fault_cpu_overload();
    test_inject_fault_bad_argument_rejected();
    test_clear_fault();
    test_status_strings();

    if (failures == 0) {
        std::cout << "All CommandProcessor tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " CommandProcessor test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
