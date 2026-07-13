#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace astra {

enum class SchedulerStatus : std::uint8_t {
    OK = 0,
    INVALID_CONFIGURATION = 1,
    TICK_DISCONTINUITY = 2
};

enum class CompletionStatus : std::uint8_t {
    COMPLETED_ON_TIME = 0,
    COMPLETED_LATE = 1,
    UNKNOWN_TASK = 2,
    NOT_PENDING = 3,
    SCHEDULER_INVALID = 4
};

struct RateGroupConfig {
    std::uint16_t task_id = 0;
    std::string name;
    std::uint32_t period_ticks = 1;
    std::uint32_t phase_ticks = 0;
    std::uint32_t relative_deadline_ticks = 1;
};

struct RateGroupRelease {
    std::uint16_t task_id = 0;
    std::uint64_t release_tick = 0;
    std::uint64_t absolute_deadline_tick = 0;
    std::uint64_t release_count = 0;
};

struct DeadlineMiss {
    std::uint16_t task_id = 0;
    std::uint64_t release_tick = 0;
    std::uint64_t deadline_tick = 0;
    std::uint64_t observed_tick = 0;
};

struct SchedulerStepResult {
    SchedulerStatus status = SchedulerStatus::OK;
    std::uint64_t tick = 0;
    std::vector<RateGroupRelease> releases;
    std::vector<DeadlineMiss> deadline_misses;
};

struct CompletionResult {
    CompletionStatus status = CompletionStatus::SCHEDULER_INVALID;
    std::uint16_t task_id = 0;
    std::uint64_t release_tick = 0;
    std::uint64_t deadline_tick = 0;
    std::uint64_t completion_tick = 0;
};

struct RateGroupStatistics {
    std::uint16_t task_id = 0;
    std::uint64_t release_count = 0;
    std::uint64_t completion_count = 0;
    std::uint64_t deadline_miss_count = 0;
    bool pending = false;
};

class RateGroupScheduler {
public:
    explicit RateGroupScheduler(std::vector<RateGroupConfig> configuration);

    bool valid() const;
    const std::string& validation_error() const;

    SchedulerStepResult step(std::uint64_t tick);
    CompletionResult complete(std::uint16_t task_id, std::uint64_t completion_tick);

    std::vector<RateGroupStatistics> statistics() const;

private:
    struct TaskState {
        RateGroupConfig config;
        bool pending = false;
        bool deadline_miss_recorded = false;
        std::uint64_t release_tick = 0;
        std::uint64_t deadline_tick = 0;
        std::uint64_t release_count = 0;
        std::uint64_t completion_count = 0;
        std::uint64_t deadline_miss_count = 0;
    };

    TaskState* find_task(std::uint16_t task_id);
    const TaskState* find_task(std::uint16_t task_id) const;

    bool valid_ = false;
    std::string validation_error_;
    std::vector<TaskState> tasks_;
    bool tick_initialized_ = false;
    std::uint64_t last_tick_ = 0;
};

std::string scheduler_status_to_string(SchedulerStatus status);
std::string completion_status_to_string(CompletionStatus status);

}  // namespace astra
