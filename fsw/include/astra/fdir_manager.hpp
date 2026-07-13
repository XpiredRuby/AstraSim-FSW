#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace astra {

enum class FaultSeverity : std::uint8_t {
    NONE = 0,
    ADVISORY = 1,
    DEGRADED = 2,
    CRITICAL = 3
};

enum class FaultResponse : std::uint8_t {
    NONE = 0,
    HOLD_CURRENT_MODE = 1,
    ENTER_DEGRADED_SENSOR = 2,
    ENTER_DEGRADED_PAYLOAD = 3,
    ENTER_SAFE = 4
};

struct FaultDisposition {
    FaultCode fault = FaultCode::NONE;
    FaultSeverity severity = FaultSeverity::NONE;
    FaultResponse response = FaultResponse::NONE;
    std::uint8_t priority = 0;
    bool latched = false;
    bool operator_clear_required = false;
    const char* detection_source = "none";
    const char* recovery_rule = "none";
};

struct FdirActionResult {
    FaultDisposition disposition;
    Mode mode_before = Mode::BOOT;
    Mode requested_mode = Mode::BOOT;
    Mode mode_after = Mode::BOOT;
    bool transition_attempted = false;
    bool transition_succeeded = false;
    bool safe_fallback_used = false;
};

class FdirManager {
public:
    FaultDisposition disposition_for(FaultCode fault) const;
    FaultCode select_primary_fault(const std::vector<FaultCode>& active_faults) const;
    FdirActionResult apply_fault(ModeManager& mode_manager, FaultCode fault) const;
};

std::string fault_severity_to_string(FaultSeverity severity);
std::string fault_response_to_string(FaultResponse response);

}  // namespace astra
