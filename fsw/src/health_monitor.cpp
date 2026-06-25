#include "astra/health_monitor.hpp"

namespace astra {
namespace {

void set_warning_if_ok(HealthReport& report, FaultCode fault, const std::string& message) {
    if (report.status == HealthStatus::OK) {
        report.status = HealthStatus::WARNING;
        report.fault = fault;
        report.message = message;
    }
}

void set_critical(HealthReport& report, FaultCode fault, const std::string& message) {
    report.status = HealthStatus::CRITICAL;
    report.fault = fault;
    report.message = message;
}

}  // namespace

std::string health_status_to_string(HealthStatus status) {
    switch (status) {
        case HealthStatus::OK:
            return "OK";
        case HealthStatus::WARNING:
            return "WARNING";
        case HealthStatus::CRITICAL:
            return "CRITICAL";
    }

    return "UNKNOWN_HEALTH_STATUS";
}

HealthMonitor::HealthMonitor(HealthThresholds thresholds)
    : thresholds_(thresholds) {}

HealthReport HealthMonitor::evaluate(const HealthSnapshot& snapshot) const {
    HealthReport report;
    report.status = HealthStatus::OK;
    report.fault = FaultCode::NONE;
    report.message = "All monitored systems nominal";

    if (snapshot.cpu_load_percent >= thresholds_.cpu_warning_percent) {
        report.cpu_ok = false;
        set_warning_if_ok(report, FaultCode::CPU_OVERLOAD, "CPU load warning threshold exceeded");
    }

    if (snapshot.cpu_load_percent >= thresholds_.cpu_critical_percent) {
        report.cpu_ok = false;
        set_critical(report, FaultCode::CPU_OVERLOAD, "CPU load critical threshold exceeded");
    }

    if (snapshot.memory_load_percent >= thresholds_.memory_warning_percent) {
        report.memory_ok = false;
        set_warning_if_ok(report, FaultCode::MEMORY_OVERLOAD, "Memory load warning threshold exceeded");
    }

    if (snapshot.memory_load_percent >= thresholds_.memory_critical_percent) {
        report.memory_ok = false;
        set_critical(report, FaultCode::MEMORY_OVERLOAD, "Memory load critical threshold exceeded");
    }

    if (snapshot.payload_age_ms >= thresholds_.payload_timeout_ms) {
        report.payload_ok = false;
        set_warning_if_ok(
            report,
            FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT,
            "Payload heartbeat timeout threshold exceeded"
        );
    }

    if (snapshot.sensor_age_ms >= thresholds_.sensor_timeout_ms) {
        report.sensor_ok = false;
        set_critical(report, FaultCode::SENSOR_TIMEOUT, "Sensor timeout threshold exceeded");
    }

    if (snapshot.loop_duration_ms >= thresholds_.loop_deadline_ms) {
        report.loop_timing_ok = false;
        set_critical(
            report,
            FaultCode::WATCHDOG_DEADLINE_MISS,
            "Flight software loop deadline missed"
        );
    }

    return report;
}

const HealthThresholds& HealthMonitor::thresholds() const {
    return thresholds_;
}

}  // namespace astra
