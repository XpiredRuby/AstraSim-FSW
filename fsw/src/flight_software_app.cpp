#include "astra/flight_software_app.hpp"

namespace astra {

FlightSoftwareApp::FlightSoftwareApp()
    : mode_manager_(),
      command_processor_(mode_manager_),
      telemetry_sequence_(0) {}

FlightSoftwareStepOutput FlightSoftwareApp::step(const FlightSoftwareStepInput& input) {
    FlightSoftwareStepOutput output;

    if (input.has_command) {
        output.command_processed = true;
        output.command_result = command_processor_.process(input.command);
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

}  // namespace astra
