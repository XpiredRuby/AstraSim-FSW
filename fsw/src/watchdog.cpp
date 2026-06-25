#include "astra/watchdog.hpp"

namespace astra {

std::string watchdog_status_to_string(WatchdogStatus status) {
    switch (status) {
        case WatchdogStatus::OK:
            return "OK";
        case WatchdogStatus::WARNING:
            return "WARNING";
        case WatchdogStatus::EXPIRED:
            return "EXPIRED";
    }

    return "UNKNOWN_WATCHDOG_STATUS";
}

Watchdog::Watchdog(WatchdogConfig config)
    : config_(config),
      last_kick_ms_(0),
      missed_deadline_count_(0) {}

void Watchdog::kick(std::uint64_t now_ms) {
    last_kick_ms_ = now_ms;
}

WatchdogReport Watchdog::evaluate(std::uint64_t now_ms, std::uint32_t loop_duration_ms) {
    WatchdogReport report;
    report.status = WatchdogStatus::OK;
    report.fault = FaultCode::NONE;
    report.message = "Watchdog nominal";

    if (loop_duration_ms >= config_.loop_deadline_ms) {
        missed_deadline_count_ += 1;
    }

    const std::uint64_t elapsed =
        now_ms >= last_kick_ms_ ? now_ms - last_kick_ms_ : 0;

    report.last_kick_ms = last_kick_ms_;
    report.elapsed_since_kick_ms = elapsed;
    report.missed_deadline_count = missed_deadline_count_;

    if (loop_duration_ms >= config_.loop_deadline_ms) {
        report.loop_timing_ok = false;
        report.status = WatchdogStatus::WARNING;
        report.message = "Loop deadline missed";
    }

    if (elapsed >= config_.warning_ms && elapsed < config_.timeout_ms) {
        report.status = WatchdogStatus::WARNING;
        report.message = "Watchdog kick warning threshold exceeded";
    }

    if (missed_deadline_count_ >= config_.max_missed_deadlines) {
        report.status = WatchdogStatus::EXPIRED;
        report.fault = FaultCode::WATCHDOG_DEADLINE_MISS;
        report.message = "Maximum missed loop deadlines exceeded";
        report.loop_timing_ok = false;
    }

    if (elapsed >= config_.timeout_ms) {
        report.status = WatchdogStatus::EXPIRED;
        report.fault = FaultCode::WATCHDOG_DEADLINE_MISS;
        report.message = "Watchdog timeout expired";
        report.alive = false;
    }

    return report;
}

void Watchdog::reset(std::uint64_t now_ms) {
    last_kick_ms_ = now_ms;
    missed_deadline_count_ = 0;
}

const WatchdogConfig& Watchdog::config() const {
    return config_;
}

std::uint64_t Watchdog::last_kick_ms() const {
    return last_kick_ms_;
}

std::uint32_t Watchdog::missed_deadline_count() const {
    return missed_deadline_count_;
}

}  // namespace astra
