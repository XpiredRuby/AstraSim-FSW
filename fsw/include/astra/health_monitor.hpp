#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>

namespace astra {

enum class HealthStatus : std::uint8_t {
    OK = 0,
    WARNING = 1,
    CRITICAL = 2
};

struct HealthThresholds {
    float cpu_warning_percent = 80.0F;
    float cpu_critical_percent = 95.0F;
    float memory_warning_percent = 85.0F;
    float memory_critical_percent = 95.0F;
    std::uint32_t sensor_timeout_ms = 1000;
    std::uint32_t payload_timeout_ms = 1500;
    std::uint32_t loop_deadline_ms = 250;
};

struct HealthSnapshot {
    float cpu_load_percent = 0.0F;
    float memory_load_percent = 0.0F;
    std::uint32_t sensor_age_ms = 0;
    std::uint32_t payload_age_ms = 0;
    std::uint32_t loop_duration_ms = 0;
};

struct HealthReport {
    HealthStatus status = HealthStatus::OK;
    FaultCode fault = FaultCode::NONE;
    std::string message;
    bool cpu_ok = true;
    bool memory_ok = true;
    bool sensor_ok = true;
    bool payload_ok = true;
    bool loop_timing_ok = true;
};

std::string health_status_to_string(HealthStatus status);

class HealthMonitor {
public:
    explicit HealthMonitor(HealthThresholds thresholds = HealthThresholds{});

    HealthReport evaluate(const HealthSnapshot& snapshot) const;

    const HealthThresholds& thresholds() const;

private:
    HealthThresholds thresholds_;
};

}  // namespace astra
