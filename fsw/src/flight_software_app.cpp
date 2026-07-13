#include "astra/flight_software_app.hpp"

#include <vector>

namespace astra {
namespace {

constexpr std::uint32_t SERIAL_HALF_RANGE = 0x80000000U;

bool sequence_is_newer(std::uint32_t candidate, std::uint32_t reference) {
    const std::uint32_t forward_distance = candidate - reference;
    return forward_distance != 0U && forward_distance < SERIAL_HALF_RANGE;
}

}  // namespace

FlightSoftwareApp::FlightSoftwareApp()
    : mode_manager_(),
      command_processor_(mode_manager_),
      fdir_manager_(),
      health_monitor_(),
      watchdog_(),
      watchdog_initialized_(false),
      telemetry_sequence_(0),
      ground_command_sequence_initialized_(false),
      highest_ground_command_sequence_number_(0),
      last_ground_command_sequence_number_(0),
      last_ground_command_id_(0),
      last_ground_command_status_(0) {}

FlightSoftwareStepOutput FlightSoftwareApp::step(const FlightSoftwareStepInput& input) {
    FlightSoftwareStepOutput output;

    if (!watchdog_initialized_) {
        watchdog_.reset(input.timestamp_ms);
        watchdog_initialized_ = true;
    }

    output.watchdog_report = watchdog_.evaluate(input.timestamp_ms, input.loop_duration_ms);

    HealthSnapshot health_snapshot;
    health_snapshot.cpu_load_percent = input.cpu_load_percent;
    health_snapshot.memory_load_percent = input.memory_load_percent;
    health_snapshot.sensor_age_ms = input.sensor_age_ms;
    health_snapshot.payload_age_ms = input.payload_age_ms;
    health_snapshot.loop_duration_ms = input.loop_duration_ms;
    output.health_report = health_monitor_.evaluate(health_snapshot);

    if (input.has_command) {
        output.command_processed = true;

        if (!ground_command_sequence_initialized_) {
            ground_command_sequence_initialized_ = true;
            highest_ground_command_sequence_number_ = input.command.sequence_number;
            output.command_result = command_processor_.process(input.command);
        } else if (input.command.sequence_number == highest_ground_command_sequence_number_) {
            output.command_result = reject_ground_command(
                input.command,
                CommandStatus::REJECTED_DUPLICATE_SEQUENCE,
                "Command rejected: duplicate ground sequence"
            );
        } else if (!sequence_is_newer(
                       input.command.sequence_number,
                       highest_ground_command_sequence_number_
                   )) {
            output.command_result = reject_ground_command(
                input.command,
                CommandStatus::REJECTED_REPLAYED_SEQUENCE,
                "Command rejected: replayed or stale ground sequence"
            );
        } else {
            highest_ground_command_sequence_number_ = input.command.sequence_number;
            output.command_result = command_processor_.process(input.command);
        }

        last_ground_command_sequence_number_ = output.command_result.sequence_number;
        last_ground_command_id_ = static_cast<std::uint8_t>(output.command_result.command_id);
        last_ground_command_status_ = static_cast<std::uint8_t>(output.command_result.status);
    }

    std::vector<FaultCode> internal_fault_candidates;
    const bool watchdog_fault_active =
        output.watchdog_report.status == WatchdogStatus::EXPIRED &&
        output.watchdog_report.fault != FaultCode::NONE;
    const bool health_fault_active =
        output.health_report.status == HealthStatus::CRITICAL &&
        output.health_report.fault != FaultCode::NONE;

    if (watchdog_fault_active) {
        internal_fault_candidates.push_back(output.watchdog_report.fault);
    }
    if (health_fault_active) {
        internal_fault_candidates.push_back(output.health_report.fault);
    }

    output.primary_internal_fault =
        fdir_manager_.select_primary_fault(internal_fault_candidates);

    if (output.primary_internal_fault != FaultCode::NONE) {
        const auto internal_fault_command =
            make_internal_fault_command(output.primary_internal_fault, input.timestamp_ms);
        output.internal_fault_processed = true;
        output.internal_fault_result = command_processor_.process(internal_fault_command);

        if (watchdog_fault_active &&
            output.watchdog_report.fault == output.primary_internal_fault) {
            output.watchdog_fault_processed = true;
            output.watchdog_fault_result = output.internal_fault_result;
        } else if (health_fault_active &&
                   output.health_report.fault == output.primary_internal_fault) {
            output.health_fault_processed = true;
            output.health_fault_result = output.internal_fault_result;
        }
    }

    watchdog_.kick(input.timestamp_ms);

    telemetry_sequence_ += 1;

    output.telemetry.sequence_number = telemetry_sequence_;
    output.telemetry.timestamp_ms = input.timestamp_ms;
    output.telemetry.mode = mode_manager_.current_mode();
    output.telemetry.last_fault = command_processor_.last_fault();
    output.telemetry.cpu_load_percent = input.cpu_load_percent;
    output.telemetry.memory_load_percent = input.memory_load_percent;
    output.telemetry.heartbeat_count = input.heartbeat_count;
    output.telemetry.last_command_sequence_number = last_ground_command_sequence_number_;
    output.telemetry.last_command_id = last_ground_command_id_;
    output.telemetry.last_command_status = last_ground_command_status_;

    return output;
}

Mode FlightSoftwareApp::current_mode() const {
    return mode_manager_.current_mode();
}

FaultCode FlightSoftwareApp::last_fault() const {
    return command_processor_.last_fault();
}

CommandPacket FlightSoftwareApp::make_internal_fault_command(
    FaultCode fault,
    std::uint64_t timestamp_ms
) const {
    CommandPacket packet;
    packet.sequence_number = 0;
    packet.timestamp_ms = timestamp_ms;
    packet.command_id = CommandId::INJECT_FAULT;
    packet.argument = static_cast<std::uint32_t>(fault);
    return packet;
}

CommandResult FlightSoftwareApp::reject_ground_command(
    const CommandPacket& packet,
    CommandStatus status,
    const char* message
) const {
    CommandResult result;
    result.status = status;
    result.command_id = packet.command_id;
    result.sequence_number = packet.sequence_number;
    result.resulting_mode = mode_manager_.current_mode();
    result.resulting_fault = command_processor_.last_fault();
    result.message = message;
    return result;
}

}  // namespace astra
