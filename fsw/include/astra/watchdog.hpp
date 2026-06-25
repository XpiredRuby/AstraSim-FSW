#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>

namespace astra {

enum class WatchdogStatus : std::uint8_t {
    OK = 0,
    WARNING = 1,
    EXPIRED = 2
};

struct WatchdogConfig {
    std::uint32_t warning_ms = 400;
    std::uint32_t timeout_ms = 500;
    std::uint32_t loop_deadline_ms = 250;
    std::uint32_t max_missed_deadlines = 3;
};

struct WatchdogReport {
    WatchdogStatus status = WatchdogStatus::OK;
    FaultCode fault = FaultCode::NONE;
    std::string message;

    std::uint64_t last_kick_ms = 0;
    std::uint64_t elapsed_since_kick_ms = 0;
    std::uint32_t missed_deadline_count = 0;

    bool alive = true;
    bool loop_timing_ok = true;
};

std::string watchdog_status_to_string(WatchdogStatus status);

class Watchdog {
public:
    explicit Watchdog(WatchdogConfig config = WatchdogConfig{});

    void kick(std::uint64_t now_ms);
    WatchdogReport evaluate(std::uint64_t now_ms, std::uint32_t loop_duration_ms);

    void reset(std::uint64_t now_ms);

    const WatchdogConfig& config() const;
    std::uint64_t last_kick_ms() const;
    std::uint32_t missed_deadline_count() const;

private:
    WatchdogConfig config_;
    std::uint64_t last_kick_ms_;
    std::uint32_t missed_deadline_count_;
};

}  // namespace astra
