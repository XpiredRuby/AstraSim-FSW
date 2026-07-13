#include "astra/recovery_supervisor.hpp"

namespace astra {

RecoverySupervisor::RecoverySupervisor(RecoverySupervisorConfig configuration)
    : configuration_(configuration) {
    if (configuration_.maximum_failed_attempts == 0U) {
        validation_error_ = "maximum failed recovery attempts must be greater than zero";
        return;
    }

    valid_ = true;
}

bool RecoverySupervisor::valid() const {
    return valid_;
}

const std::string& RecoverySupervisor::validation_error() const {
    return validation_error_;
}

void RecoverySupervisor::begin_session() {
    failed_attempts_ = 0U;
}

RecoveryAttemptResult RecoverySupervisor::record_success() {
    RecoveryAttemptResult result;
    result.maximum_failed_attempts = configuration_.maximum_failed_attempts;

    if (!valid_) {
        result.status = RecoveryAttemptStatus::INVALID_CONFIGURATION;
        result.failed_attempts = failed_attempts_;
        return result;
    }

    failed_attempts_ = 0U;
    result.status = RecoveryAttemptStatus::SUCCESS;
    return result;
}

RecoveryAttemptResult RecoverySupervisor::record_failure(
    ModeManager& mode_manager
) {
    RecoveryAttemptResult result;
    result.maximum_failed_attempts = configuration_.maximum_failed_attempts;

    if (!valid_) {
        result.status = RecoveryAttemptStatus::INVALID_CONFIGURATION;
        result.failed_attempts = failed_attempts_;
        return result;
    }

    failed_attempts_ += 1U;
    result.failed_attempts = failed_attempts_;

    if (failed_attempts_ >= configuration_.maximum_failed_attempts) {
        result.fail_safe_forced = mode_manager.transition_to(
            Mode::SAFE,
            FaultCode::NONE
        );
        result.status = result.fail_safe_forced
            ? RecoveryAttemptStatus::FAIL_SAFE_FORCED
            : RecoveryAttemptStatus::FAILED;
        return result;
    }

    result.status = RecoveryAttemptStatus::FAILED;
    return result;
}

std::uint32_t RecoverySupervisor::failed_attempts() const {
    return failed_attempts_;
}

std::string recovery_attempt_status_to_string(RecoveryAttemptStatus status) {
    switch (status) {
        case RecoveryAttemptStatus::SUCCESS:
            return "SUCCESS";
        case RecoveryAttemptStatus::FAILED:
            return "FAILED";
        case RecoveryAttemptStatus::FAIL_SAFE_FORCED:
            return "FAIL_SAFE_FORCED";
        case RecoveryAttemptStatus::INVALID_CONFIGURATION:
            return "INVALID_CONFIGURATION";
    }

    return "UNKNOWN_RECOVERY_ATTEMPT_STATUS";
}

}  // namespace astra
