#pragma once

#include <cstdint>
#include <string>

namespace astra {

enum class GroundCommandGuardStatus : std::uint8_t {
    ACCEPTED = 0,
    REJECTED_DUPLICATE_SEQUENCE = 1,
    REJECTED_REPLAYED_SEQUENCE = 2,
    REJECTED_STALE_TIMESTAMP = 3,
    REJECTED_FUTURE_TIMESTAMP = 4
};

struct GroundCommandGuardConfig {
    std::uint64_t maximum_age_ms = 5000U;
    std::uint64_t maximum_future_skew_ms = 1000U;
};

struct GroundCommandGuardResult {
    GroundCommandGuardStatus status = GroundCommandGuardStatus::ACCEPTED;
    bool sequence_consumed = false;
    bool sequence_initialized = false;
    std::uint32_t highest_sequence = 0;
    std::uint64_t age_ms = 0;
    std::uint64_t future_skew_ms = 0;
};

class GroundCommandGuard {
public:
    explicit GroundCommandGuard(
        GroundCommandGuardConfig configuration = GroundCommandGuardConfig{}
    );

    bool valid() const;
    const std::string& validation_error() const;

    GroundCommandGuardResult evaluate(
        std::uint32_t sequence_number,
        std::uint64_t command_timestamp_ms,
        std::uint64_t mission_time_ms
    );

    bool sequence_initialized() const;
    std::uint32_t highest_sequence() const;

private:
    GroundCommandGuardConfig configuration_;
    bool valid_ = false;
    std::string validation_error_;
    bool sequence_initialized_ = false;
    std::uint32_t highest_sequence_ = 0;
};

std::string ground_command_guard_status_to_string(
    GroundCommandGuardStatus status
);

}  // namespace astra
