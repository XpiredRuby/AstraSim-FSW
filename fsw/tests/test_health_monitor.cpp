#include "astra/health_monitor.hpp"

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

astra::HealthSnapshot nominal_snapshot() {
    astra::HealthSnapshot snapshot;
    snapshot.cpu_load_percent = 20.0F;
    snapshot.memory_load_percent = 40.0F;
    snapshot.sensor_age_ms = 10;
    snapshot.payload_age_ms = 20;
    snapshot.loop_duration_ms = 25;
    return snapshot;
}

void test_nominal_snapshot_is_ok() {
    astra::HealthMonitor monitor;
    const auto report = monitor.evaluate(nominal_snapshot());

    expect_true(report.status == astra::HealthStatus::OK, "nominal snapshot reports OK");
    expect_true(report.fault == astra::FaultCode::NONE, "nominal snapshot has no fault");
    expect_true(report.cpu_ok, "nominal CPU is OK");
    expect_true(report.memory_ok, "nominal memory is OK");
    expect_true(report.sensor_ok, "nominal sensor is OK");
    expect_true(report.payload_ok, "nominal payload is OK");
    expect_true(report.loop_timing_ok, "nominal loop timing is OK");
}

void test_cpu_warning() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.cpu_load_percent = 85.0F;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::WARNING, "high CPU reports WARNING");
    expect_true(report.fault == astra::FaultCode::CPU_OVERLOAD, "high CPU maps to CPU_OVERLOAD");
    expect_true(!report.cpu_ok, "high CPU marks cpu_ok false");
}

void test_cpu_critical() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.cpu_load_percent = 97.0F;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::CRITICAL, "critical CPU reports CRITICAL");
    expect_true(report.fault == astra::FaultCode::CPU_OVERLOAD, "critical CPU maps to CPU_OVERLOAD");
}

void test_memory_warning() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.memory_load_percent = 88.0F;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::WARNING, "high memory reports WARNING");
    expect_true(report.fault == astra::FaultCode::MEMORY_OVERLOAD, "high memory maps to MEMORY_OVERLOAD");
    expect_true(!report.memory_ok, "high memory marks memory_ok false");
}

void test_memory_critical() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.memory_load_percent = 98.0F;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::CRITICAL, "critical memory reports CRITICAL");
    expect_true(report.fault == astra::FaultCode::MEMORY_OVERLOAD, "critical memory maps to MEMORY_OVERLOAD");
}

void test_payload_timeout_warning() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.payload_age_ms = 2000;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::WARNING, "payload timeout reports WARNING");
    expect_true(
        report.fault == astra::FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT,
        "payload timeout maps to PAYLOAD_HEARTBEAT_TIMEOUT"
    );
    expect_true(!report.payload_ok, "payload timeout marks payload_ok false");
}

void test_sensor_timeout_critical() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.sensor_age_ms = 1500;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::CRITICAL, "sensor timeout reports CRITICAL");
    expect_true(report.fault == astra::FaultCode::SENSOR_TIMEOUT, "sensor timeout maps to SENSOR_TIMEOUT");
    expect_true(!report.sensor_ok, "sensor timeout marks sensor_ok false");
}

void test_loop_deadline_miss_critical() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.loop_duration_ms = 300;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::CRITICAL, "loop deadline miss reports CRITICAL");
    expect_true(
        report.fault == astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        "loop deadline miss maps to WATCHDOG_DEADLINE_MISS"
    );
    expect_true(!report.loop_timing_ok, "loop deadline miss marks loop_timing_ok false");
}

void test_critical_overrides_warning() {
    astra::HealthMonitor monitor;
    auto snapshot = nominal_snapshot();
    snapshot.cpu_load_percent = 85.0F;
    snapshot.sensor_age_ms = 1500;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::CRITICAL, "critical fault overrides warning");
    expect_true(report.fault == astra::FaultCode::SENSOR_TIMEOUT, "critical sensor fault is reported");
    expect_true(!report.cpu_ok, "warning condition still recorded");
    expect_true(!report.sensor_ok, "critical condition recorded");
}

void test_custom_thresholds() {
    astra::HealthThresholds thresholds;
    thresholds.cpu_warning_percent = 50.0F;
    thresholds.cpu_critical_percent = 90.0F;

    astra::HealthMonitor monitor(thresholds);
    auto snapshot = nominal_snapshot();
    snapshot.cpu_load_percent = 60.0F;

    const auto report = monitor.evaluate(snapshot);

    expect_true(report.status == astra::HealthStatus::WARNING, "custom CPU warning threshold works");
    expect_true(monitor.thresholds().cpu_warning_percent == 50.0F, "custom thresholds stored");
}

void test_status_strings() {
    expect_true(
        astra::health_status_to_string(astra::HealthStatus::CRITICAL) == "CRITICAL",
        "health status string conversion works"
    );
}

}  // namespace

int main() {
    std::cout << "Running HealthMonitor tests..." << std::endl;

    test_nominal_snapshot_is_ok();
    test_cpu_warning();
    test_cpu_critical();
    test_memory_warning();
    test_memory_critical();
    test_payload_timeout_warning();
    test_sensor_timeout_critical();
    test_loop_deadline_miss_critical();
    test_critical_overrides_warning();
    test_custom_thresholds();
    test_status_strings();

    if (failures == 0) {
        std::cout << "All HealthMonitor tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " HealthMonitor test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
