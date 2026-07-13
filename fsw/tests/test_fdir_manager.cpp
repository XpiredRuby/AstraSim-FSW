#include "astra/fdir_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

void move_to_nominal(astra::ModeManager& manager) {
    static_cast<void>(manager.transition_to(astra::Mode::NOMINAL, astra::FaultCode::NONE));
}

void test_all_faults_have_explicit_dispositions() {
    const astra::FdirManager fdir;
    const std::vector<astra::FaultCode> faults = {
        astra::FaultCode::SENSOR_TIMEOUT,
        astra::FaultCode::SENSOR_INVALID_DATA,
        astra::FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT,
        astra::FaultCode::CPU_OVERLOAD,
        astra::FaultCode::MEMORY_OVERLOAD,
        astra::FaultCode::TELEMETRY_SOCKET_FAILURE,
        astra::FaultCode::COMMAND_BAD_CRC,
        astra::FaultCode::COMMAND_UNKNOWN_ID,
        astra::FaultCode::COMMAND_TIMEOUT,
        astra::FaultCode::WATCHDOG_DEADLINE_MISS,
    };

    for (const auto fault : faults) {
        const auto disposition = fdir.disposition_for(fault);
        expect_true(disposition.fault == fault, "fault disposition preserves fault identity");
        expect_true(disposition.severity != astra::FaultSeverity::NONE, "fault has non-NONE severity");
        expect_true(disposition.response != astra::FaultResponse::NONE, "fault has explicit response");
        expect_true(disposition.priority > 0U, "fault has nonzero priority");
        expect_true(std::string(disposition.detection_source) != "none", "fault has detection source");
        expect_true(std::string(disposition.recovery_rule) != "none", "fault has recovery rule");
    }
}

void test_advisory_command_faults_hold_mode() {
    const astra::FdirManager fdir;
    astra::ModeManager manager;
    move_to_nominal(manager);

    const auto bad_crc = fdir.apply_fault(manager, astra::FaultCode::COMMAND_BAD_CRC);
    const auto unknown = fdir.apply_fault(manager, astra::FaultCode::COMMAND_UNKNOWN_ID);

    expect_true(!bad_crc.transition_attempted, "bad CRC does not request mode transition");
    expect_true(!unknown.transition_attempted, "unknown command does not request mode transition");
    expect_true(manager.current_mode() == astra::Mode::NOMINAL, "advisory command faults hold NOMINAL");
}

void test_payload_fault_degrades_nominal() {
    const astra::FdirManager fdir;
    astra::ModeManager manager;
    move_to_nominal(manager);

    const auto result = fdir.apply_fault(manager, astra::FaultCode::CPU_OVERLOAD);

    expect_true(result.transition_attempted, "CPU overload requests transition");
    expect_true(result.transition_succeeded, "CPU overload transition succeeds");
    expect_true(!result.safe_fallback_used, "NOMINAL CPU overload needs no fallback");
    expect_true(manager.current_mode() == astra::Mode::DEGRADED_PAYLOAD, "CPU overload enters degraded payload");
}

void test_degradation_fault_falls_back_safe_from_test() {
    const astra::FdirManager fdir;
    astra::ModeManager manager;
    manager.transition_to(astra::Mode::STANDBY, astra::FaultCode::NONE);
    manager.transition_to(astra::Mode::TEST, astra::FaultCode::NONE);

    const auto result = fdir.apply_fault(manager, astra::FaultCode::CPU_OVERLOAD);

    expect_true(result.safe_fallback_used, "invalid TEST degradation uses safe fallback");
    expect_true(result.transition_succeeded, "safe fallback succeeds");
    expect_true(manager.current_mode() == astra::Mode::SAFE, "TEST CPU overload enters SAFE");
}

void test_critical_faults_enter_safe() {
    const astra::FdirManager fdir;
    const std::vector<astra::FaultCode> critical_faults = {
        astra::FaultCode::SENSOR_TIMEOUT,
        astra::FaultCode::SENSOR_INVALID_DATA,
        astra::FaultCode::MEMORY_OVERLOAD,
        astra::FaultCode::TELEMETRY_SOCKET_FAILURE,
        astra::FaultCode::COMMAND_TIMEOUT,
        astra::FaultCode::WATCHDOG_DEADLINE_MISS,
    };

    for (const auto fault : critical_faults) {
        astra::ModeManager manager;
        move_to_nominal(manager);
        const auto result = fdir.apply_fault(manager, fault);
        expect_true(result.transition_succeeded, "critical fault SAFE transition succeeds");
        expect_true(manager.current_mode() == astra::Mode::SAFE, "critical fault enters SAFE");
    }
}

void test_primary_fault_selection_is_order_independent() {
    const astra::FdirManager fdir;
    const auto first = fdir.select_primary_fault({
        astra::FaultCode::CPU_OVERLOAD,
        astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        astra::FaultCode::SENSOR_TIMEOUT,
    });
    const auto second = fdir.select_primary_fault({
        astra::FaultCode::SENSOR_TIMEOUT,
        astra::FaultCode::CPU_OVERLOAD,
        astra::FaultCode::WATCHDOG_DEADLINE_MISS,
    });

    expect_true(first == astra::FaultCode::WATCHDOG_DEADLINE_MISS, "watchdog wins simultaneous fault priority");
    expect_true(second == first, "fault selection is independent of observation order");
}

void test_none_is_ignored_during_selection() {
    const astra::FdirManager fdir;
    const auto selected = fdir.select_primary_fault({
        astra::FaultCode::NONE,
        astra::FaultCode::CPU_OVERLOAD,
        astra::FaultCode::NONE,
    });

    expect_true(selected == astra::FaultCode::CPU_OVERLOAD, "NONE observations are ignored");
    expect_true(
        fdir.select_primary_fault({astra::FaultCode::NONE}) == astra::FaultCode::NONE,
        "all-NONE observations select NONE"
    );
}

void test_status_strings_are_stable() {
    expect_true(
        astra::fault_severity_to_string(astra::FaultSeverity::CRITICAL) == "CRITICAL",
        "critical severity string is stable"
    );
    expect_true(
        astra::fault_response_to_string(astra::FaultResponse::ENTER_SAFE) == "ENTER_SAFE",
        "safe response string is stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running FdirManager tests..." << std::endl;

    test_all_faults_have_explicit_dispositions();
    test_advisory_command_faults_hold_mode();
    test_payload_fault_degrades_nominal();
    test_degradation_fault_falls_back_safe_from_test();
    test_critical_faults_enter_safe();
    test_primary_fault_selection_is_order_independent();
    test_none_is_ignored_during_selection();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All FdirManager tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " FdirManager test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
