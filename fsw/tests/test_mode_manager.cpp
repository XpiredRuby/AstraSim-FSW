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

void test_safe_to_nominal_rejected() {
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::SAFE, astra::FaultCode::SENSOR_TIMEOUT);

    const bool ok = manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE);

    expect_true(!ok, "SAFE -> NOMINAL direct transition rejected");
    expect_mode(manager.current_mode(), astra::Mode::SAFE, "mode stays SAFE");
}

}  // namespace

int main() {
    std::cout << "Running ModeManager tests..." << std::endl;

    test_initial_mode_is_boot();
    test_boot_to_nominal_transition_allowed();
    test_boot_to_degraded_payload_rejected();
    test_cpu_overload_moves_nominal_to_degraded_payload();
    test_sensor_timeout_moves_nominal_to_safe();
    test_safe_to_nominal_rejected();

    if (failures == 0) {
        std::cout << "All ModeManager tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " ModeManager test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
