#include "astra/event_logger.hpp"

#include <utility>

namespace astra {

EventLogger::EventLogger(std::size_t capacity)
    : capacity_(capacity) {
    records_.reserve(capacity_);
}

bool EventLogger::valid() const {
    return capacity_ > 0U;
}

std::size_t EventLogger::capacity() const {
    return capacity_;
}

std::size_t EventLogger::size() const {
    return records_.size();
}

std::uint64_t EventLogger::dropped_count() const {
    return dropped_count_;
}

std::uint64_t EventLogger::log(
    std::uint64_t timestamp_ms,
    EventSeverity severity,
    EventSource source,
    std::uint16_t code,
    std::string message
) {
    if (!valid()) {
        return 0U;
    }

    EventRecord record;
    record.sequence = next_sequence_;
    record.timestamp_ms = timestamp_ms;
    record.severity = severity;
    record.source = source;
    record.code = code;
    record.message = std::move(message);
    next_sequence_ += 1U;

    if (records_.size() < capacity_) {
        records_.push_back(std::move(record));
        return next_sequence_ - 1U;
    }

    records_.at(start_index_) = std::move(record);
    start_index_ = (start_index_ + 1U) % capacity_;
    dropped_count_ += 1U;
    return next_sequence_ - 1U;
}

std::vector<EventRecord> EventLogger::snapshot() const {
    std::vector<EventRecord> output;
    output.reserve(records_.size());

    if (records_.empty()) {
        return output;
    }

    for (std::size_t offset = 0; offset < records_.size(); ++offset) {
        const std::size_t index = (start_index_ + offset) % records_.size();
        output.push_back(records_.at(index));
    }

    return output;
}

void EventLogger::clear() {
    records_.clear();
    start_index_ = 0U;
    dropped_count_ = 0U;
}

std::string event_severity_to_string(EventSeverity severity) {
    switch (severity) {
        case EventSeverity::DEBUG:
            return "DEBUG";
        case EventSeverity::INFO:
            return "INFO";
        case EventSeverity::WARNING:
            return "WARNING";
        case EventSeverity::ERROR:
            return "ERROR";
        case EventSeverity::CRITICAL:
            return "CRITICAL";
    }

    return "UNKNOWN_SEVERITY";
}

std::string event_source_to_string(EventSource source) {
    switch (source) {
        case EventSource::SYSTEM:
            return "SYSTEM";
        case EventSource::COMMAND:
            return "COMMAND";
        case EventSource::MODE:
            return "MODE";
        case EventSource::HEALTH:
            return "HEALTH";
        case EventSource::WATCHDOG:
            return "WATCHDOG";
        case EventSource::FDIR:
            return "FDIR";
        case EventSource::SCHEDULER:
            return "SCHEDULER";
        case EventSource::TELEMETRY:
            return "TELEMETRY";
        case EventSource::CONFIGURATION:
            return "CONFIGURATION";
    }

    return "UNKNOWN_SOURCE";
}

}  // namespace astra
