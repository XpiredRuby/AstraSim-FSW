#include "astra/rate_group_scheduler.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

astra::RateGroupConfig make_config(
    std::uint16_t task_id,
    const std::string& name,
    std::uint32_t period,
    std::uint32_t phase,
    std::uint32_t deadline
) {
    astra::RateGroupConfig config;
    config.task_id = task_id;
    config.name = name;
    config.period_ticks = period;
    config.phase_ticks = phase;
    config.relative_deadline_ticks = deadline;
    return config;
}

void test_valid_configuration_is_accepted() {
    astra::RateGroupScheduler scheduler({
        make_config(1, "fast", 1, 0, 1),
        make_config(2, "slow", 4, 1, 2)
    });

    expect_true(scheduler.valid(), "valid rate-group configuration accepted");
    expect_true(scheduler.validation_error().empty(), "valid scheduler has no validation error");
}

void test_invalid_configurations_are_rejected() {
    astra::RateGroupScheduler empty({});
    astra::RateGroupScheduler zero_period({make_config(1, "bad", 0, 0, 1)});
    astra::RateGroupScheduler bad_phase({make_config(1, "bad", 4, 4, 1)});
    astra::RateGroupScheduler zero_deadline({make_config(1, "bad", 4, 0, 0)});
    astra::RateGroupScheduler long_deadline({make_config(1, "bad", 4, 0, 5)});
    astra::RateGroupScheduler duplicate_ids({
        make_config(7, "first", 1, 0, 1),
        make_config(7, "second", 2, 0, 1)
    });

    expect_true(!empty.valid(), "empty scheduler configuration rejected");
    expect_true(!zero_period.valid(), "zero period rejected");
    expect_true(!bad_phase.valid(), "phase outside period rejected");
    expect_true(!zero_deadline.valid(), "zero deadline rejected");
    expect_true(!long_deadline.valid(), "deadline longer than period rejected");
    expect_true(!duplicate_ids.valid(), "duplicate task IDs rejected");
}

void test_release_pattern_and_completion() {
    astra::RateGroupScheduler scheduler({
        make_config(1, "fast", 1, 0, 1),
        make_config(2, "slow", 2, 1, 1)
    });

    const auto tick0 = scheduler.step(0);
    expect_true(tick0.status == astra::SchedulerStatus::OK, "tick zero accepted");
    expect_true(tick0.releases.size() == 1U, "tick zero releases only fast group");
    expect_true(tick0.releases.at(0).task_id == 1U, "fast group releases at tick zero");
    expect_true(
        scheduler.complete(1, 0).status == astra::CompletionStatus::COMPLETED_ON_TIME,
        "fast group completes on time"
    );

    const auto tick1 = scheduler.step(1);
    expect_true(tick1.releases.size() == 2U, "tick one releases fast and phased slow groups");
    expect_true(tick1.releases.at(0).task_id == 1U, "fast group remains first deterministically");
    expect_true(tick1.releases.at(1).task_id == 2U, "slow group releases at phase tick");
    scheduler.complete(1, 1);
    scheduler.complete(2, 2);

    const auto tick2 = scheduler.step(2);
    expect_true(tick2.releases.size() == 1U, "tick two releases only fast group");
    expect_true(tick2.releases.at(0).release_count == 3U, "fast release count increments deterministically");
}

void test_tick_discontinuity_is_rejected_without_advancing_state() {
    astra::RateGroupScheduler scheduler({make_config(1, "fast", 1, 0, 1)});

    scheduler.step(0);
    scheduler.complete(1, 0);
    const auto skipped = scheduler.step(2);
    const auto recovered = scheduler.step(1);

    expect_true(
        skipped.status == astra::SchedulerStatus::TICK_DISCONTINUITY,
        "skipped scheduler tick rejected"
    );
    expect_true(recovered.status == astra::SchedulerStatus::OK, "expected tick remains acceptable after rejection");
}

void test_deadline_miss_is_detected_once() {
    astra::RateGroupScheduler scheduler({make_config(1, "task", 3, 0, 1)});

    scheduler.step(0);
    const auto tick1 = scheduler.step(1);
    const auto tick2 = scheduler.step(2);
    const auto completion = scheduler.complete(1, 2);
    const auto stats = scheduler.statistics();

    expect_true(tick1.deadline_misses.empty(), "deadline is not missed at its exact tick");
    expect_true(tick2.deadline_misses.size() == 1U, "late pending task produces one deadline miss");
    expect_true(
        completion.status == astra::CompletionStatus::COMPLETED_LATE,
        "completion after recorded deadline is late"
    );
    expect_true(stats.at(0).deadline_miss_count == 1U, "late completion does not double-count miss");
}

void test_pending_job_at_next_release_is_an_overrun() {
    astra::RateGroupScheduler scheduler({make_config(1, "task", 2, 0, 2)});

    scheduler.step(0);
    scheduler.step(1);
    const auto tick2 = scheduler.step(2);
    const auto stats = scheduler.statistics();

    expect_true(tick2.deadline_misses.size() == 1U, "pending job at next release records overrun miss");
    expect_true(tick2.releases.size() == 1U, "new release still occurs after overrun accounting");
    expect_true(stats.at(0).release_count == 2U, "overrun task receives deterministic next release");
    expect_true(stats.at(0).deadline_miss_count == 1U, "overrun increments deadline miss count");
}

void test_completion_errors_are_explicit() {
    astra::RateGroupScheduler scheduler({make_config(1, "task", 2, 0, 1)});

    const auto unknown = scheduler.complete(99, 0);
    const auto not_pending = scheduler.complete(1, 0);

    expect_true(unknown.status == astra::CompletionStatus::UNKNOWN_TASK, "unknown task completion rejected");
    expect_true(not_pending.status == astra::CompletionStatus::NOT_PENDING, "non-pending completion rejected");
}

void test_two_instances_produce_identical_release_streams() {
    const std::vector<astra::RateGroupConfig> configuration = {
        make_config(1, "one", 1, 0, 1),
        make_config(2, "two", 3, 1, 2),
        make_config(3, "three", 5, 2, 3)
    };

    astra::RateGroupScheduler first(configuration);
    astra::RateGroupScheduler second(configuration);
    bool identical = true;

    for (std::uint64_t tick = 0; tick < 20U; ++tick) {
        const auto first_result = first.step(tick);
        const auto second_result = second.step(tick);

        identical = identical && first_result.status == second_result.status;
        identical = identical && first_result.releases.size() == second_result.releases.size();

        for (std::size_t index = 0; index < first_result.releases.size(); ++index) {
            const auto& left = first_result.releases.at(index);
            const auto& right = second_result.releases.at(index);
            identical = identical && left.task_id == right.task_id;
            identical = identical && left.release_tick == right.release_tick;
            identical = identical && left.absolute_deadline_tick == right.absolute_deadline_tick;
            identical = identical && left.release_count == right.release_count;

            first.complete(left.task_id, tick);
            second.complete(right.task_id, tick);
        }
    }

    expect_true(identical, "identical scheduler inputs produce identical release streams");
}

void test_status_strings_are_stable() {
    expect_true(
        astra::scheduler_status_to_string(astra::SchedulerStatus::TICK_DISCONTINUITY) ==
            "TICK_DISCONTINUITY",
        "scheduler status string is stable"
    );
    expect_true(
        astra::completion_status_to_string(astra::CompletionStatus::COMPLETED_LATE) ==
            "COMPLETED_LATE",
        "completion status string is stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running RateGroupScheduler tests..." << std::endl;

    test_valid_configuration_is_accepted();
    test_invalid_configurations_are_rejected();
    test_release_pattern_and_completion();
    test_tick_discontinuity_is_rejected_without_advancing_state();
    test_deadline_miss_is_detected_once();
    test_pending_job_at_next_release_is_an_overrun();
    test_completion_errors_are_explicit();
    test_two_instances_produce_identical_release_streams();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All RateGroupScheduler tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " RateGroupScheduler test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
