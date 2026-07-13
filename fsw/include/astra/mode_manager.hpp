#pragma once

#include <cstdint>
#include <string>

namespace astra {

enum class Mode : std::uint8_t {
    BOOT = 0,
    NOMINAL = 1,
    DEGRADED_SENSOR = 2,
    DEGRADED_PAYLOAD = 3,
    SAFE = 4,
    RECOVERY = 5,
    STANDBY = 6,
    TEST = 7
};

enum class FaultCode : std::uint16_t {
    NONE = 0,
    SENSOR_TIMEOUT = 100,
    SENSOR_INVALID_DATA = 101,
    PAYLOAD_HEARTBEAT_TIMEOUT = 200,
    CPU_OVERLOAD = 300,
    MEMORY_OVERLOAD = 301,
    TELEMETRY_SOCKET_FAILURE = 400,
    COMMAND_BAD_CRC = 500,
    COMMAND_UNKNOWN_ID = 501,
    COMMAND_TIMEOUT = 502,
    WATCHDOG_DEADLINE_MISS = 600
};

std::string mode_to_string(Mode mode);
std::string fault_to_string(FaultCode fault);

class ModeManager {
public:
    ModeManager();

    Mode current_mode() const;
    bool transition_to(Mode next_mode, FaultCode reason);
    void handle_fault(FaultCode fault);

private:
    Mode mode_;

    bool is_valid_transition(Mode from, Mode to) const;
};

}  // namespace astra
