#include "astra/event_logger.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect_true(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        ++failures;
    }
}

void test_zero_capacity_is_invalid() {
    astra::EventLogger logger(0U);
    expect_true(!logger.valid(), "zero-capacity logger is invalid");
    expect_true(
        logger.log(1U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 1U, "ignored") == 0U,
        "invalid logger rejects event"
    );
    expect_true(logger.snapshot().empty(), "invalid logger remains empty");
}

void test_events_are_sequenced_and_ordered() {
    astra::EventLogger logger(3U);
    const auto first = logger.log(100U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 1U, "boot");
    const auto second = logger.log(110U, astra::EventSeverity::WARNING, astra::EventSource::HEALTH, 2U, "warning");
    const auto snapshot = logger.snapshot();

    expect_true(first == 1U, "first event sequence is one");
    expect_true(second == 2U, "second event sequence is two");
    expect_true(snapshot.size() == 2U, "two events retained");
    expect_true(snapshot.at(0).message == "boot", "first event remains first");
    expect_true(snapshot.at(1).message == "warning", "second event remains second");
    expect_true(logger.dropped_count() == 0U, "no events dropped below capacity");
}

void test_overflow_retains_newest_events_in_order() {
    astra::EventLogger logger(3U);
    for (std::uint64_t value = 1U; value <= 5U; ++value) {
        logger.log(
            100U + value,
            astra::EventSeverity::INFO,
            astra::EventSource::SYSTEM,
            static_cast<std::uint16_t>(value),
            "event-" + std::to_string(value)
        );
    }

    const auto snapshot = logger.snapshot();
    expect_true(snapshot.size() == 3U, "overflow keeps bounded capacity");
    expect_true(logger.dropped_count() == 2U, "overflow count records two replacements");
    expect_true(snapshot.at(0).sequence == 3U, "oldest retained event has sequence three");
    expect_true(snapshot.at(1).sequence == 4U, "middle retained event has sequence four");
    expect_true(snapshot.at(2).sequence == 5U, "newest retained event has sequence five");
    expect_true(snapshot.at(0).message == "event-3", "snapshot begins with oldest retained message");
    expect_true(snapshot.at(2).message == "event-5", "snapshot ends with newest message");
}

void test_clear_resets_storage_but_not_sequence_identity() {
    astra::EventLogger logger(2U);
    logger.log(1U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 1U, "first");
    logger.log(2U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 2U, "second");
    logger.log(3U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 3U, "third");
    logger.clear();

    expect_true(logger.size() == 0U, "clear removes retained records");
    expect_true(logger.dropped_count() == 0U, "clear resets overflow count");
    const auto next = logger.log(4U, astra::EventSeverity::INFO, astra::EventSource::SYSTEM, 4U, "after-clear");
    expect_true(next == 4U, "clear does not reuse prior event sequence numbers");
}

void test_record_fields_and_strings() {
    astra::EventLogger logger(1U);
    logger.log(
        55U,
        astra::EventSeverity::CRITICAL,
        astra::EventSource::FDIR,
        600U,
        "watchdog"
    );
    const auto record = logger.snapshot().at(0);

    expect_true(record.timestamp_ms == 55U, "event timestamp retained");
    expect_true(record.severity == astra::EventSeverity::CRITICAL, "event severity retained");
    expect_true(record.source == astra::EventSource::FDIR, "event source retained");
    expect_true(record.code == 600U, "event code retained");
    expect_true(
        astra::event_severity_to_string(record.severity) == "CRITICAL",
        "severity string is stable"
    );
    expect_true(
        astra::event_source_to_string(record.source) == "FDIR",
        "source string is stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running EventLogger tests..." << std::endl;

    test_zero_capacity_is_invalid();
    test_events_are_sequenced_and_ordered();
    test_overflow_retains_newest_events_in_order();
    test_clear_resets_storage_but_not_sequence_identity();
    test_record_fields_and_strings();

    if (failures == 0) {
        std::cout << "All EventLogger tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " EventLogger test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
