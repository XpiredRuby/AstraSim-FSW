#pragma once

#include "astra/command_packet.hpp"
#include "astra/command_processor.hpp"
#include "astra/event_logger.hpp"
#include "astra/fdir_manager.hpp"
#include "astra/ground_command_guard.hpp"
#include "astra/health_monitor.hpp"
#include "astra/mode_manager.hpp"
#include "astra/telemetry_packet.hpp"
#include "astra/watchdog.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace astra {

struct FlightSoftwareAppConfig {
    GroundCommandGuardConfig ground_command_guard;
    std::size_t event_log_capacity = 256U;
};

struct FlightSoftwareStepInput {
    bool has_command = false;
    CommandPacket command;

    std::uint64_t timestamp_ms = 0;
    float cpu_load_percent = 0.0F;
    float memory_load_percent = 0.0F;
    std::uint32_t heartbeat_count = 0;

    std::uint32_t sensor_age_ms = 0;
    std::uint32_t payload_age_ms = 0;
    std::uint32_t loop_duration_ms = 0;
};

struct FlightSoftwareStepOutput {
    bool command_processed = false;
    GroundCommandGuardResult command_guard_result;
    CommandResult command_result;

    WatchdogReport watchdog_report;
    bool watchdog_fault_processed = false;
    CommandResult watchdog_fault_result;

    HealthReport health_report;
    bool health_fault_processed = false;
    CommandResult health_fault_result;

    bool internal_fault_processed = false;
    FaultCode primary_internal_fault = FaultCode::NONE;
    CommandResult internal_fault_result;

    TelemetryPacket telemetry;
};

class FlightSoftwareApp {
public:
    explicit FlightSoftwareApp(
        FlightSoftwareAppConfig configuration = FlightSoftwareAppConfig{}
    );

    bool valid() const;
    const std::string& validation_error() const;

    FlightSoftwareStepOutput step(const FlightSoftwareStepInput& input);

    Mode current_mode() const;
    FaultCode last_fault() const;
    std::vector<EventRecord> event_snapshot() const;
    std::uint64_t dropped_event_count() const;

private:
    CommandPacket make_internal_fault_command(
        FaultCode fault,
        std::uint64_t timestamp_ms
    ) const;

    CommandResult reject_ground_command(
        const CommandPacket& packet,
        CommandStatus status,
        const std::string& message
    ) const;

    void record_step_events(
        std::uint64_t timestamp_ms,
        Mode mode_before,
        FaultCode fault_before,
        const FlightSoftwareStepOutput& output
    );

    ModeManager mode_manager_;
    CommandProcessor command_processor_;
    FdirManager fdir_manager_;
    GroundCommandGuard ground_command_guard_;
    HealthMonitor health_monitor_;
    Watchdog watchdog_;
    EventLogger event_logger_;
    bool watchdog_initialized_;
    std::uint32_t telemetry_sequence_;
    std::uint32_t last_ground_command_sequence_number_;
    std::uint8_t last_ground_command_id_;
    std::uint8_t last_ground_command_status_;
    bool valid_ = false;
    std::string validation_error_;
};

}  // namespace astra
