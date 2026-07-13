#include "astra/ground_command_guard.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
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

void test_default_configuration_accepts_current_command() {
    astra::GroundCommandGuard guard;
    const auto result = guard.evaluate(1U, 1000U, 1000U);

    expect_true(guard.valid(), "default guard configuration valid");
    expect_true(result.status == astra::GroundCommandGuardStatus::ACCEPTED, "current command accepted");
    expect_true(result.sequence_consumed, "accepted command consumes sequence");
    expect_true(result.highest_sequence == 1U, "accepted sequence becomes highest");
}

void test_age_and_future_boundaries_are_inclusive() {
    astra::GroundCommandGuardConfig config;
    config.maximum_age_ms = 100U;
    config.maximum_future_skew_ms = 20U;

    astra::GroundCommandGuard age_guard(config);
    const auto age_boundary = age_guard.evaluate(1U, 900U, 1000U);
    expect_true(age_boundary.status == astra::GroundCommandGuardStatus::ACCEPTED, "maximum age boundary accepted");
    expect_true(age_boundary.age_ms == 100U, "accepted age reported exactly");

    astra::GroundCommandGuard future_guard(config);
    const auto future_boundary = future_guard.evaluate(1U, 1020U, 1000U);
    expect_true(future_boundary.status == astra::GroundCommandGuardStatus::ACCEPTED, "maximum future skew boundary accepted");
    expect_true(future_boundary.future_skew_ms == 20U, "accepted future skew reported exactly");
}

void test_stale_and_future_commands_do_not_consume_sequence() {
    astra::GroundCommandGuardConfig config;
    config.maximum_age_ms = 100U;
    config.maximum_future_skew_ms = 20U;
    astra::GroundCommandGuard guard(config);

    const auto stale = guard.evaluate(100U, 899U, 1000U);
    const auto future = guard.evaluate(101U, 1021U, 1000U);
    const auto corrected = guard.evaluate(1U, 1000U, 1000U);

    expect_true(stale.status == astra::GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP, "stale command rejected");
    expect_true(stale.age_ms == 101U, "stale age reported");
    expect_true(!stale.sequence_consumed, "stale command does not consume sequence");
    expect_true(future.status == astra::GroundCommandGuardStatus::REJECTED_FUTURE_TIMESTAMP, "future command rejected");
    expect_true(future.future_skew_ms == 21U, "future skew reported");
    expect_true(!future.sequence_consumed, "future command does not consume sequence");
    expect_true(corrected.status == astra::GroundCommandGuardStatus::ACCEPTED, "corrected first sequence accepted");
    expect_true(corrected.highest_sequence == 1U, "rejected timestamps did not alter sequence state");
}

void test_duplicate_and_replay_rejection() {
    astra::GroundCommandGuard guard;
    static_cast<void>(guard.evaluate(10U, 1000U, 1000U));

    const auto duplicate = guard.evaluate(10U, 1001U, 1001U);
    const auto replay = guard.evaluate(9U, 1002U, 1002U);
    const auto newer = guard.evaluate(11U, 1003U, 1003U);

    expect_true(duplicate.status == astra::GroundCommandGuardStatus::REJECTED_DUPLICATE_SEQUENCE, "duplicate rejected");
    expect_true(replay.status == astra::GroundCommandGuardStatus::REJECTED_REPLAYED_SEQUENCE, "older sequence rejected");
    expect_true(newer.status == astra::GroundCommandGuardStatus::ACCEPTED, "newer sequence accepted");
    expect_true(guard.highest_sequence() == 11U, "highest sequence advances only on accepted command");
}

void test_sequence_wrap_and_ambiguous_half_range() {
    astra::GroundCommandGuard guard;
    const auto maximum = std::numeric_limits<std::uint32_t>::max();
    static_cast<void>(guard.evaluate(maximum, 1000U, 1000U));
    const auto wrapped = guard.evaluate(0U, 1001U, 1001U);

    expect_true(wrapped.status == astra::GroundCommandGuardStatus::ACCEPTED, "maximum to zero wrap accepted");

    astra::GroundCommandGuard ambiguous_guard;
    static_cast<void>(ambiguous_guard.evaluate(0U, 1000U, 1000U));
    const auto ambiguous = ambiguous_guard.evaluate(0x80000000U, 1001U, 1001U);
    expect_true(
        ambiguous.status == astra::GroundCommandGuardStatus::REJECTED_REPLAYED_SEQUENCE,
        "exact half-range jump rejected as ambiguous"
    );
}

void test_invalid_configuration_is_explicit() {
    astra::GroundCommandGuardConfig config;
    config.maximum_age_ms = std::numeric_limits<std::uint64_t>::max();
    astra::GroundCommandGuard guard(config);
    const auto result = guard.evaluate(1U, 1000U, 1000U);

    expect_true(!guard.valid(), "unbounded maximum age rejected");
    expect_true(!guard.validation_error().empty(), "invalid guard provides diagnostic");
    expect_true(
        result.status == astra::GroundCommandGuardStatus::INVALID_CONFIGURATION,
        "invalid guard cannot accept commands"
    );
}

void test_status_strings_are_stable() {
    expect_true(
        astra::ground_command_guard_status_to_string(
            astra::GroundCommandGuardStatus::REJECTED_STALE_TIMESTAMP
        ) == "REJECTED_STALE_TIMESTAMP",
        "stale timestamp status string stable"
    );
    expect_true(
        astra::ground_command_guard_status_to_string(
            astra::GroundCommandGuardStatus::REJECTED_FUTURE_TIMESTAMP
        ) == "REJECTED_FUTURE_TIMESTAMP",
        "future timestamp status string stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running GroundCommandGuard tests..." << std::endl;

    test_default_configuration_accepts_current_command();
    test_age_and_future_boundaries_are_inclusive();
    test_stale_and_future_commands_do_not_consume_sequence();
    test_duplicate_and_replay_rejection();
    test_sequence_wrap_and_ambiguous_half_range();
    test_invalid_configuration_is_explicit();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All GroundCommandGuard tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " GroundCommandGuard test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
