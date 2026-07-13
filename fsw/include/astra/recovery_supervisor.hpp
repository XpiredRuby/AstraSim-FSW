#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>

namespace astra {

struct RecoverySupervisorConfig {
    std::uint32_t maximum_failed_attempts = 3U;
};

enum class RecoveryAttemptStatus : std::uint8_t {
    SUCCESS = 0,
    FAILED = 1,
    FAIL_SAFE_FORCED = 2,
    INVALID_CONFIGURATION = 3
};

struct RecoveryAttemptResult {
    RecoveryAttemptStatus status = RecoveryAttemptStatus::SUCCESS;
    std::uint32_t failed_attempts = 0U;
    std::uint32_t maximum_failed_attempts = 0U;
    bool fail_safe_forced = false;
};

class RecoverySupervisor {
public:
    explicit RecoverySupervisor(
        RecoverySupervisorConfig configuration = RecoverySupervisorConfig{}
    );

    bool valid() const;
    const std::string& validation_error() const;

    void begin_session();
    RecoveryAttemptResult record_success();
    RecoveryAttemptResult record_failure(ModeManager& mode_manager);

    std::uint32_t failed_attempts() const;

private:
    RecoverySupervisorConfig configuration_;
    bool valid_ = false;
    std::string validation_error_;
    std::uint32_t failed_attempts_ = 0U;
};

std::string recovery_attempt_status_to_string(RecoveryAttemptStatus status);

}  // namespace astra
