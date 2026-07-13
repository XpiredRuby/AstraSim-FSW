#pragma once

#include "astra/flight_software_app.hpp"
#include "astra/rate_group_scheduler.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace astra {

enum class ExecutiveTaskId : std::uint16_t {
    FLIGHT_SOFTWARE_STEP = 1,
    HOUSEKEEPING_SNAPSHOT = 2
};

struct FlightSoftwareExecutiveConfig {
    FlightSoftwareAppConfig app;
    std::uint32_t flight_period_ticks = 1;
    std::uint32_t flight_phase_ticks = 0;
    std::uint32_t flight_deadline_ticks = 1;
    std::uint32_t housekeeping_period_ticks = 10;
    std::uint32_t housekeeping_phase_ticks = 0;
    std::uint32_t housekeeping_deadline_ticks = 2;
};

struct FlightSoftwareExecutiveInput {
    std::uint64_t tick = 0;
    FlightSoftwareStepInput flight_input;
    std::uint32_t simulated_flight_completion_delay_ticks = 0;
};

struct FlightSoftwareExecutiveOutput {
    SchedulerStatus scheduler_status = SchedulerStatus::INVALID_CONFIGURATION;
    std::vector<RateGroupRelease> releases;
    std::vector<DeadlineMiss> deadline_misses;

    bool flight_software_ran = false;
    FlightSoftwareStepOutput flight_output;
    CompletionResult flight_completion;

    bool housekeeping_ran = false;
    CompletionResult housekeeping_completion;
    std::vector<RateGroupStatistics> scheduler_statistics;
};

class FlightSoftwareExecutive {
public:
    explicit FlightSoftwareExecutive(
        FlightSoftwareExecutiveConfig configuration = FlightSoftwareExecutiveConfig{}
    );

    bool valid() const;
    const std::string& validation_error() const;

    FlightSoftwareExecutiveOutput step(const FlightSoftwareExecutiveInput& input);

    Mode current_mode() const;
    FaultCode last_fault() const;

private:
    static std::vector<RateGroupConfig> make_scheduler_configuration(
        const FlightSoftwareExecutiveConfig& configuration
    );

    RateGroupScheduler scheduler_;
    FlightSoftwareApp app_;
};

}  // namespace astra
