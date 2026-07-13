#include "astra/rate_group_scheduler.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace astra {

RateGroupScheduler::RateGroupScheduler(std::vector<RateGroupConfig> configuration) {
    if (configuration.empty()) {
        validation_error_ = "scheduler requires at least one rate group";
        return;
    }

    tasks_.reserve(configuration.size());

    for (auto& config : configuration) {
        if (config.name.empty()) {
            validation_error_ = "rate-group name must not be empty";
            tasks_.clear();
            return;
        }

        if (config.period_ticks == 0U) {
            validation_error_ = "rate-group period must be greater than zero";
            tasks_.clear();
            return;
        }

        if (config.phase_ticks >= config.period_ticks) {
            validation_error_ = "rate-group phase must be less than its period";
            tasks_.clear();
            return;
        }

        if (config.relative_deadline_ticks == 0U ||
            config.relative_deadline_ticks > config.period_ticks) {
            validation_error_ = "rate-group deadline must be in the range [1, period]";
            tasks_.clear();
            return;
        }

        const bool duplicate_id = std::any_of(
            tasks_.begin(),
            tasks_.end(),
            [&config](const TaskState& task) {
                return task.config.task_id == config.task_id;
            }
        );

        if (duplicate_id) {
            validation_error_ = "rate-group task IDs must be unique";
            tasks_.clear();
            return;
        }

        TaskState state;
        state.config = std::move(config);
        tasks_.push_back(std::move(state));
    }

    valid_ = true;
}

bool RateGroupScheduler::valid() const {
    return valid_;
}

const std::string& RateGroupScheduler::validation_error() const {
    return validation_error_;
}

SchedulerStepResult RateGroupScheduler::step(std::uint64_t tick) {
    SchedulerStepResult result;
    result.tick = tick;

    if (!valid_) {
        result.status = SchedulerStatus::INVALID_CONFIGURATION;
        return result;
    }

    if (!tick_initialized_) {
        if (tick != 0U) {
            result.status = SchedulerStatus::TICK_DISCONTINUITY;
            return result;
        }

        tick_initialized_ = true;
    } else if (last_tick_ == std::numeric_limits<std::uint64_t>::max() ||
               tick != last_tick_ + 1U) {
        result.status = SchedulerStatus::TICK_DISCONTINUITY;
        return result;
    }

    for (auto& task : tasks_) {
        if (task.pending && !task.deadline_miss_recorded && tick > task.deadline_tick) {
            task.deadline_miss_recorded = true;
            task.deadline_miss_count += 1U;

            DeadlineMiss miss;
            miss.task_id = task.config.task_id;
            miss.release_tick = task.release_tick;
            miss.deadline_tick = task.deadline_tick;
            miss.observed_tick = tick;
            result.deadline_misses.push_back(miss);
        }

        const bool due =
            tick >= task.config.phase_ticks &&
            ((tick - task.config.phase_ticks) % task.config.period_ticks) == 0U;

        if (!due) {
            continue;
        }

        if (task.pending && !task.deadline_miss_recorded) {
            task.deadline_miss_recorded = true;
            task.deadline_miss_count += 1U;

            DeadlineMiss miss;
            miss.task_id = task.config.task_id;
            miss.release_tick = task.release_tick;
            miss.deadline_tick = task.deadline_tick;
            miss.observed_tick = tick;
            result.deadline_misses.push_back(miss);
        }

        task.pending = true;
        task.deadline_miss_recorded = false;
        task.release_tick = tick;
        task.deadline_tick = tick + task.config.relative_deadline_ticks;
        task.release_count += 1U;

        RateGroupRelease release;
        release.task_id = task.config.task_id;
        release.release_tick = task.release_tick;
        release.absolute_deadline_tick = task.deadline_tick;
        release.release_count = task.release_count;
        result.releases.push_back(release);
    }

    last_tick_ = tick;
    result.status = SchedulerStatus::OK;
    return result;
}

CompletionResult RateGroupScheduler::complete(
    std::uint16_t task_id,
    std::uint64_t completion_tick
) {
    CompletionResult result;
    result.task_id = task_id;
    result.completion_tick = completion_tick;

    if (!valid_) {
        result.status = CompletionStatus::SCHEDULER_INVALID;
        return result;
    }

    TaskState* task = find_task(task_id);
    if (task == nullptr) {
        result.status = CompletionStatus::UNKNOWN_TASK;
        return result;
    }

    if (!task->pending) {
        result.status = CompletionStatus::NOT_PENDING;
        return result;
    }

    result.release_tick = task->release_tick;
    result.deadline_tick = task->deadline_tick;
    task->completion_count += 1U;

    const bool completed_on_time =
        completion_tick >= task->release_tick &&
        completion_tick <= task->deadline_tick &&
        !task->deadline_miss_recorded;

    if (completed_on_time) {
        result.status = CompletionStatus::COMPLETED_ON_TIME;
    } else {
        result.status = CompletionStatus::COMPLETED_LATE;

        if (!task->deadline_miss_recorded) {
            task->deadline_miss_recorded = true;
            task->deadline_miss_count += 1U;
        }
    }

    task->pending = false;
    task->deadline_miss_recorded = false;
    return result;
}

std::vector<RateGroupStatistics> RateGroupScheduler::statistics() const {
    std::vector<RateGroupStatistics> output;
    output.reserve(tasks_.size());

    for (const auto& task : tasks_) {
        RateGroupStatistics statistics;
        statistics.task_id = task.config.task_id;
        statistics.release_count = task.release_count;
        statistics.completion_count = task.completion_count;
        statistics.deadline_miss_count = task.deadline_miss_count;
        statistics.pending = task.pending;
        output.push_back(statistics);
    }

    return output;
}

RateGroupScheduler::TaskState* RateGroupScheduler::find_task(std::uint16_t task_id) {
    const auto iterator = std::find_if(
        tasks_.begin(),
        tasks_.end(),
        [task_id](const TaskState& task) {
            return task.config.task_id == task_id;
        }
    );

    return iterator == tasks_.end() ? nullptr : &(*iterator);
}

const RateGroupScheduler::TaskState* RateGroupScheduler::find_task(
    std::uint16_t task_id
) const {
    const auto iterator = std::find_if(
        tasks_.begin(),
        tasks_.end(),
        [task_id](const TaskState& task) {
            return task.config.task_id == task_id;
        }
    );

    return iterator == tasks_.end() ? nullptr : &(*iterator);
}

std::string scheduler_status_to_string(SchedulerStatus status) {
    switch (status) {
        case SchedulerStatus::OK:
            return "OK";
        case SchedulerStatus::INVALID_CONFIGURATION:
            return "INVALID_CONFIGURATION";
        case SchedulerStatus::TICK_DISCONTINUITY:
            return "TICK_DISCONTINUITY";
    }

    return "UNKNOWN_SCHEDULER_STATUS";
}

std::string completion_status_to_string(CompletionStatus status) {
    switch (status) {
        case CompletionStatus::COMPLETED_ON_TIME:
            return "COMPLETED_ON_TIME";
        case CompletionStatus::COMPLETED_LATE:
            return "COMPLETED_LATE";
        case CompletionStatus::UNKNOWN_TASK:
            return "UNKNOWN_TASK";
        case CompletionStatus::NOT_PENDING:
            return "NOT_PENDING";
        case CompletionStatus::SCHEDULER_INVALID:
            return "SCHEDULER_INVALID";
    }

    return "UNKNOWN_COMPLETION_STATUS";
}

}  // namespace astra
