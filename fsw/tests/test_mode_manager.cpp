#include "astra/mode_manager.hpp"

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

void expect_mode(astra::Mode actual, astra::Mode expected, const std::string& test_name) {
    const bool passed = (actual == expected);

    if (passed) {
        std::cout << "[PASS] " << test_name
                  << " | mode=" << astra::mode_to_string(actual)
                  << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name
                  << " | expected=" << astra::mode_to_string(expected)
                  << " actual=" << astra::mode_to_string(actual)
                  << std::endl;
        ++failures;
    }
}

void test_initial_mode_is_boot() {
    astra::ModeManager manager;
    expect_mode(manager.current_mode(), astra::Mode::BOOT, "initial mode is BOOT");
}

void test_boot_to_nominal_transition_allowed() {
    astra::ModeManager manager;
    const bool ok = manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);

    expect_true(ok, "BOOT -> NOMINAL transition allowed");
    expect_mode(manager.current_mode(), astra::Mode::NOMINAL, "mode becomes NOMINAL");
}

void test_boot_to_standby_transition_allowed() {
    astra::ModeManager manager;
    const bool ok = manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);

    expect_true(ok, "BOOT -> STANDBY transition allowed");
    expect_mode(manager.current_mode(), astra::Mode::STANDBY, "mode becomes STANDBY");
}

void test_standby_test_round_trip() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);

    const bool entered_test = manager.transition_to(astra::Mode::TEST, astra::FaultCode::NONE);
    const bool returned_standby = manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);

    expect_true(entered_test, "STANDBY -> TEST transition allowed");
    expect_true(returned_standby, "TEST -> STANDBY transition allowed");
    expect_mode(manager.current_mode(), astra::Mode::STANDBY, "TEST round trip returns to STANDBY");
}

void test_nominal_to_test_direct_transition_rejected() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);

    const bool ok = manager.transition_to(astra::Mode::TEST, astra::FaultCode::NONE);

    expect_true(!ok, "NOMINAL -> TEST direct transition rejected");
    expect_mode(manager.current_mode(), astra::Mode::NOMINAL, "rejected TEST entry preserves NOMINAL");
}

void test_test_to_nominal_direct_transition_rejected() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);
    manager.transition_to(astra::Mode::TEST, astra::FaultCode::NONE);

    const bool ok = manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);

    expect_true(!ok, "TEST -> NOMINAL direct transition rejected");
    expect_mode(manager.current_mode(), astra::Mode::TEST, "rejected TEST exit preserves TEST");
}

void test_boot_to_degraded_payload_rejected() {
    astra::ModeManager manager;
    const bool ok = manager.transition_to(
        astra::Mode::DEGRADED_PAYLOAD,
        astra::FaultCode::CPU_OVERLOAD
    );

    expect_true(!ok, "BOOT -> DEGRADED_PAYLOAD transition rejected");
    expect_mode(manager.current_mode(), astra::Mode::BOOT, "mode stays BOOT after invalid transition");
}

void test_cpu_overload_moves_nominal_to_degraded_payload() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);
    manager.handle_fault(astra::FaultCode::CPU_OVERLOAD);

    expect_mode(
        manager.current_mode(),
        astra::Mode::DEGRADED_PAYLOAD,
        "CPU_OVERLOAD moves NOMINAL -> DEGRADED_PAYLOAD"
    );
}

void test_sensor_timeout_moves_nominal_to_safe() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);
    manager.handle_fault(astra::FaultCode::SENSOR_TIMEOUT);

    expect_mode(
        manager.current_mode(),
        astra::Mode::SAFE,
        "SENSOR_TIMEOUT moves NOMINAL -> SAFE"
    );
}

void test_fault_in_test_moves_to_safe() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);
    manager.transition_to(astra::Mode::TEST, astra::FaultCode::NONE);
    manager.handle_fault(astra::FaultCode::WATCHDOG_DEADLINE_MISS);

    expect_mode(
        manager.current_mode(),
        astra::Mode::SAFE,
        "critical watchdog fault moves TEST -> SAFE"
    );
}

void test_safe_to_nominal_rejected() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::SAFE, astra::FaultCode::SENSOR_TIMEOUT);

    const bool ok = manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);

    expect_true(!ok, "SAFE -> NOMINAL direct transition rejected");
    expect_mode(manager.current_mode(), astra::Mode::SAFE, "mode stays SAFE");
}

void test_mode_numeric_values_remain_stable() {
    expect_true(static_cast<std::uint8_t>(astra::Mode::BOOT) == 0U, "BOOT numeric value remains 0");
    expect_true(static_cast<std::uint8_t>(astra::Mode::NOMINAL) == 1U, "NOMINAL numeric value remains 1");
    expect_true(static_cast<std::uint8_t>(astra::Mode::DEGRADED_SENSOR) == 2U, "DEGRADED_SENSOR numeric value remains 2");
    expect_true(static_cast<std::uint8_t>(astra::Mode::DEGRADED_PAYLOAD) == 3U, "DEGRADED_PAYLOAD numeric value remains 3");
    expect_true(static_cast<std::uint8_t>(astra::Mode::SAFE) == 4U, "SAFE numeric value remains 4");
    expect_true(static_cast<std::uint8_t>(astra::Mode::RECOVERY) == 5U, "RECOVERY numeric value remains 5");
    expect_true(static_cast<std::uint8_t>(astra::Mode::STANDBY) == 6U, "STANDBY numeric value is appended as 6");
    expect_true(static_cast<std::uint8_t>(astra::Mode::TEST) == 7U, "TEST numeric value is appended as 7");
}

void test_mode_strings_are_stable() {
    expect_true(astra::mode_to_string(astra::Mode::STANDBY) == "STANDBY", "STANDBY string is stable");
    expect_true(astra::mode_to_string(astra::Mode::TEST) == "TEST", "TEST string is stable");
}

}  // namespace

int main() {
    std::cout << "Running ModeManager tests..." << std::endl;

    test_initial_mode_is_boot();
    test_boot_to_nominal_transition_allowed();
    test_boot_to_standby_transition_allowed();
    test_standby_test_round_trip();
    test_nominal_to_test_direct_transition_rejected();
    test_test_to_nominal_direct_transition_rejected();
    test_boot_to_degraded_payload_rejected();
    test_cpu_overload_moves_nominal_to_degraded_payload();
    test_sensor_timeout_moves_nominal_to_safe();
    test_fault_in_test_moves_to_safe();
    test_safe_to_nominal_rejected();
    test_mode_numeric_values_remain_stable();
    test_mode_strings_are_stable();

    if (failures == 0) {
        std::cout << "All ModeManager tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " ModeManager test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
