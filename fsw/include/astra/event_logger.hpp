#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace astra {

enum class EventSeverity : std::uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

enum class EventSource : std::uint8_t {
    SYSTEM = 0,
    COMMAND = 1,
    MODE = 2,
    HEALTH = 3,
    WATCHDOG = 4,
    FDIR = 5,
    SCHEDULER = 6,
    TELEMETRY = 7,
    CONFIGURATION = 8
};

struct EventRecord {
    std::uint64_t sequence = 0;
    std::uint64_t timestamp_ms = 0;
    EventSeverity severity = EventSeverity::INFO;
    EventSource source = EventSource::SYSTEM;
    std::uint16_t code = 0;
    std::string message;
};

class EventLogger {
public:
    explicit EventLogger(std::size_t capacity = 256);

    bool valid() const;
    std::size_t capacity() const;
    std::size_t size() const;
    std::uint64_t dropped_count() const;

    std::uint64_t log(
        std::uint64_t timestamp_ms,
        EventSeverity severity,
        EventSource source,
        std::uint16_t code,
        std::string message
    );

    std::vector<EventRecord> snapshot() const;
    void clear();

private:
    std::size_t capacity_ = 0;
    std::vector<EventRecord> records_;
    std::size_t start_index_ = 0;
    std::uint64_t next_sequence_ = 1;
    std::uint64_t dropped_count_ = 0;
};

std::string event_severity_to_string(EventSeverity severity);
std::string event_source_to_string(EventSource source);

}  // namespace astra
