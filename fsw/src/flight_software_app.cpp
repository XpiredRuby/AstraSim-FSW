#include "astra/flight_software_app.hpp"

#include <string>
#include <vector>

namespace astra {
namespace {

EventSeverity event_severity_for_fault(const FaultDisposition& disposition) {
    switch (disposition.severity) {
        case FaultSeverity::NONE:
            return EventSeverity::INFO;
        case FaultSeverity::ADVISORY:
            return EventSeverity::WARNING;
        case FaultSeverity::DEGRADED:
            return EventSeverity::ERROR;
        case FaultSeverity::CRITICAL:
            return EventSeverity::CRITICAL;
    }

    return EventSeverity::CRITICAL;
}

}  // namespace

FlightSoftwareApp::FlightSoftwareApp(FlightSoftwareAppConfig configuration)
    : mode_manager_(),
      command_processor_(mode_manager_),
      fdir_manager_(),
      ground_command_guard_(configuration.ground_command_guard),
      health_monitor_(),
      watchdog_(),
      event_logger_(configuration.event_log_capacity),
      watchdog_initialized_(false),
      telemetry_sequence_(0),
      last_ground_command_sequence_number_(0),
      last_ground_command_id_(0),
      last_ground_command_status_(0) {
    if (!ground_command_guard_.valid()) {
        validation_error_ = ground_command_guard_.validation_error();
        return;
    }

    if (!event_logger_.valid()) {
        validation_error_ = "event log capacity must be greater than zero";
        return;
    }

    valid_ = true;
}

bool FlightSoftwareApp::valid() const {
    return valid_;
}

const std::string& FlightSoftwareApp::validation_error() const {
    return validation_error_;
}

FlightSoftwareStepOutput FlightSoftwareApp::step(const FlightSoftwareStepInput& input) {
    FlightSoftwareStepOutput output;
    const Mode mode_before = mode_manager_.current_mode();
    const FaultCode fault_before = command_processor_.last_fault();

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
        output.command_guard_result = ground_command_guard_.evaluate(
            input.command.sequence_number,
            input.command.timestamp_ms,
            input.timestamp_ms
        );

        switch (output.command_guard_result.status) {
            case GroundCommandGuardStatus::ACCEPTED:
                output.command_result = command_processor_.process(input.command);
                break;

            case GroundCommandGuardStatus::REJECTED_DUPLICATE_SEQUENCE:
                output.command_result = reject_ground_command(
                    input.command,
                    CommandStatus::REJECTED_DUPLICATE_SEQUENCE,
                    "Command rejected: duplicate ground sequence"
                );
                break;

            case GroundCommandGuardStatus::REJECTED_REPLAYED_SEQUENCE:
                output.command_result = reject_ground_command(
                    input.command,
                    CommandStatus::REJECTED_REPLAYED_SEQUENCE,
                    "Command rejected: replayed or ambiguous ground sequence"
                );
                break;

            case GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP:
                output.command_result = reject_ground_command(
                    input.command,
                    CommandStatus::REJECTED_STALE_TIMESTAMP,
                    "Command rejected: stale timestamp age_ms=" +
                        std::to_string(output.command_guard_result.age_ms)
                );
                break;

            case GroundCommandGuardStatus::REJECTED_FUTURE_TIMESTAMP:
                output.command_result = reject_ground_command(
                    input.command,
                    CommandStatus::REJECTED_FUTURE_TIMESTAMP,
                    "Command rejected: future timestamp skew_ms=" +
                        std::to_string(output.command_guard_result.future_skew_ms)
                );
                break;

            case GroundCommandGuardStatus::INVALID_CONFIGURATION:
                output.command_result = reject_ground_command(
                    input.command,
                    CommandStatus::REJECTED_GUARD_CONFIGURATION,
                    "Command rejected: invalid ground command guard configuration"
                );
                break;
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
    record_step_events(input.timestamp_ms, mode_before, fault_before, output);

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

std::vector<EventRecord> FlightSoftwareApp::event_snapshot() const {
    return event_logger_.snapshot();
}

std::uint64_t FlightSoftwareApp::dropped_event_count() const {
    return event_logger_.dropped_count();
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
    const std::string& message
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

void FlightSoftwareApp::record_step_events(
    std::uint64_t timestamp_ms,
    Mode mode_before,
    FaultCode fault_before,
    const FlightSoftwareStepOutput& output
) {
    if (output.command_processed) {
        const auto severity =
            output.command_result.status == CommandStatus::ACCEPTED
                ? EventSeverity::INFO
                : EventSeverity::WARNING;
        event_logger_.log(
            timestamp_ms,
            severity,
            EventSource::COMMAND,
            static_cast<std::uint16_t>(output.command_result.command_id),
            "sequence=" + std::to_string(output.command_result.sequence_number) +
                " status=" + command_status_to_string(output.command_result.status) +
                " message=" + output.command_result.message
        );
    }

    const Mode mode_after = mode_manager_.current_mode();
    if (mode_after != mode_before) {
        event_logger_.log(
            timestamp_ms,
            EventSeverity::INFO,
            EventSource::MODE,
            static_cast<std::uint16_t>(mode_after),
            "mode=" + mode_to_string(mode_before) + "->" + mode_to_string(mode_after)
        );
    }

    const FaultCode fault_after = command_processor_.last_fault();
    if (fault_after != fault_before) {
        EventSeverity severity = EventSeverity::INFO;
        std::string response = "CLEARED";
        if (fault_after != FaultCode::NONE) {
            const auto disposition = fdir_manager_.disposition_for(fault_after);
            severity = event_severity_for_fault(disposition);
            response = fault_response_to_string(disposition.response);
        }

        event_logger_.log(
            timestamp_ms,
            severity,
            EventSource::FDIR,
            static_cast<std::uint16_t>(fault_after),
            "fault=" + fault_to_string(fault_before) + "->" +
                fault_to_string(fault_after) + " response=" + response
        );
    }
}

}  // namespace astra
