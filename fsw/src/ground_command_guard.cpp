#include "astra/ground_command_guard.hpp"

#include <limits>

namespace astra {
namespace {

constexpr std::uint32_t SERIAL_HALF_RANGE = 0x80000000U;

bool sequence_is_newer(std::uint32_t candidate, std::uint32_t reference) {
    const std::uint32_t forward_distance = candidate - reference;
    return forward_distance != 0U && forward_distance < SERIAL_HALF_RANGE;
}

}  // namespace

GroundCommandGuard::GroundCommandGuard(GroundCommandGuardConfig configuration)
    : configuration_(configuration) {
    if (configuration_.maximum_age_ms == std::numeric_limits<std::uint64_t>::max()) {
        validation_error_ = "maximum command age must leave room for bounded comparison";
        return;
    }

    if (configuration_.maximum_future_skew_ms ==
        std::numeric_limits<std::uint64_t>::max()) {
        validation_error_ = "maximum future skew must leave room for bounded comparison";
        return;
    }

    valid_ = true;
}

bool GroundCommandGuard::valid() const {
    return valid_;
}

const std::string& GroundCommandGuard::validation_error() const {
    return validation_error_;
}

GroundCommandGuardResult GroundCommandGuard::evaluate(
    std::uint32_t sequence_number,
    std::uint64_t command_timestamp_ms,
    std::uint64_t mission_time_ms
) {
    GroundCommandGuardResult result;
    result.sequence_initialized = sequence_initialized_;
    result.highest_sequence = highest_sequence_;

    if (!valid_) {
        result.status = GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP;
        return result;
    }

    if (command_timestamp_ms > mission_time_ms) {
        result.future_skew_ms = command_timestamp_ms - mission_time_ms;
        if (result.future_skew_ms > configuration_.maximum_future_skew_ms) {
            result.status = GroundCommandGuardStatus::REJECTED_FUTURE_TIMESTAMP;
            return result;
        }
    } else {
        result.age_ms = mission_time_ms - command_timestamp_ms;
        if (result.age_ms > configuration_.maximum_age_ms) {
            result.status = GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP;
            return result;
        }
    }

    if (!sequence_initialized_) {
        sequence_initialized_ = true;
        highest_sequence_ = sequence_number;
        result.status = GroundCommandGuardStatus::ACCEPTED;
        result.sequence_consumed = true;
        result.sequence_initialized = true;
        result.highest_sequence = highest_sequence_;
        return result;
    }

    if (sequence_number == highest_sequence_) {
        result.status = GroundCommandGuardStatus::REJECTED_DUPLICATE_SEQUENCE;
        return result;
    }

    if (!sequence_is_newer(sequence_number, highest_sequence_)) {
        result.status = GroundCommandGuardStatus::REJECTED_REPLAYED_SEQUENCE;
        return result;
    }

    highest_sequence_ = sequence_number;
    result.status = GroundCommandGuardStatus::ACCEPTED;
    result.sequence_consumed = true;
    result.sequence_initialized = true;
    result.highest_sequence = highest_sequence_;
    return result;
}

bool GroundCommandGuard::sequence_initialized() const {
    return sequence_initialized_;
}

std::uint32_t GroundCommandGuard::highest_sequence() const {
    return highest_sequence_;
}

std::string ground_command_guard_status_to_string(
    GroundCommandGuardStatus status
) {
    switch (status) {
        case GroundCommandGuardStatus::ACCEPTED:
            return "ACCEPTED";
        case GroundCommandGuardStatus::REJECTED_DUPLICATE_SEQUENCE:
            return "REJECTED_DUPLICATE_SEQUENCE";
        case GroundCommandGuardStatus::REJECTED_REPLAYED_SEQUENCE:
            return "REJECTED_REPLAYED_SEQUENCE";
        case GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP:
            return "REJECTED_STALE_TIMESTAMP";
        case GroundCommandGuardStatus::REJECTED_FUTURE_TIMESTAMP:
            return "REJECTED_FUTURE_TIMESTAMP";
    }

    return "UNKNOWN_GUARD_STATUS";
}

}  // namespace astra
