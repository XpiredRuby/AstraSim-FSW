#pragma once

#include "astra/command_packet.hpp"
#include "astra/command_processor.hpp"
#include "astra/mode_manager.hpp"
#include "astra/telemetry_packet.hpp"

#include <cstdint>

namespace astra {

struct FlightSoftwareStepInput {
    bool has_command = false;
    CommandPacket command;

    std::uint64_t timestamp_ms = 0;
    float cpu_load_percent = 0.0F;
    float memory_load_percent = 0.0F;
    std::uint32_t heartbeat_count = 0;
};

struct FlightSoftwareStepOutput {
    bool command_processed = false;
    CommandResult command_result;
    TelemetryPacket telemetry;
};

class FlightSoftwareApp {
public:
    FlightSoftwareApp();

    FlightSoftwareStepOutput step(const FlightSoftwareStepInput& input);

    Mode current_mode() const;
    FaultCode last_fault() const;

private:
    ModeManager mode_manager_;
    CommandProcessor command_processor_;
    std::uint32_t telemetry_sequence_;
};

}  // namespace astra
