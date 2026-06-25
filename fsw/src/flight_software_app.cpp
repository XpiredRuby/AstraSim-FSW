#include "astra/flight_software_app.hpp"

namespace astra {

FlightSoftwareApp::FlightSoftwareApp()
    : mode_manager_(),
      command_processor_(mode_manager_),
      health_monitor_(),
      telemetry_sequence_(0) {}

FlightSoftwareStepOutput FlightSoftwareApp::step(const FlightSoftwareStepInput& input) {
    FlightSoftwareStepOutput output;

    if (input.has_command) {
        output.command_processed = true;
        output.command_result = command_processor_.process(input.command);
    }

    HealthSnapshot health_snapshot;
    health_snapshot.cpu_load_percent = input.cpu_load_percent;
    health_snapshot.memory_load_percent = input.memory_load_percent;
    health_snapshot.sensor_age_ms = input.sensor_age_ms;
    health_snapshot.payload_age_ms = input.payload_age_ms;
    health_snapshot.loop_duration_ms = input.loop_duration_ms;

    output.health_report = health_monitor_.evaluate(health_snapshot);

    if (output.health_report.status == HealthStatus::CRITICAL &&
        output.health_report.fault != FaultCode::NONE) {
        const auto health_fault_command =
            make_health_fault_command(output.health_report.fault, input.timestamp_ms);

        output.health_fault_processed = true;
        output.health_fault_result = command_processor_.process(health_fault_command);
    }

    telemetry_sequence_ += 1;

    output.telemetry.sequence_number = telemetry_sequence_;
    output.telemetry.timestamp_ms = input.timestamp_ms;
    output.telemetry.mode = mode_manager_.current_mode();
    output.telemetry.last_fault = command_processor_.last_fault();
    output.telemetry.cpu_load_percent = input.cpu_load_percent;
    output.telemetry.memory_load_percent = input.memory_load_percent;
    output.telemetry.heartbeat_count = input.heartbeat_count;

    return output;
}

Mode FlightSoftwareApp::current_mode() const {
    return mode_manager_.current_mode();
}

FaultCode FlightSoftwareApp::last_fault() const {
    return command_processor_.last_fault();
}

CommandPacket FlightSoftwareApp::make_health_fault_command(
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

}  // namespace astra
