#include "astra/flight_software_executive.hpp"

#include <limits>

namespace astra {

FlightSoftwareExecutive::FlightSoftwareExecutive(
    FlightSoftwareExecutiveConfig configuration
)
    : scheduler_(make_scheduler_configuration(configuration)),
      app_() {}

bool FlightSoftwareExecutive::valid() const {
    return scheduler_.valid();
}

const std::string& FlightSoftwareExecutive::validation_error() const {
    return scheduler_.validation_error();
}

FlightSoftwareExecutiveOutput FlightSoftwareExecutive::step(
    const FlightSoftwareExecutiveInput& input
) {
    FlightSoftwareExecutiveOutput output;
    const auto scheduler_result = scheduler_.step(input.tick);
    output.scheduler_status = scheduler_result.status;
    output.releases = scheduler_result.releases;
    output.deadline_misses = scheduler_result.deadline_misses;

    if (scheduler_result.status != SchedulerStatus::OK) {
        output.scheduler_statistics = scheduler_.statistics();
        return output;
    }

    for (const auto& release : scheduler_result.releases) {
        const auto task_id = static_cast<ExecutiveTaskId>(release.task_id);

        switch (task_id) {
            case ExecutiveTaskId::FLIGHT_SOFTWARE_STEP: {
                output.flight_software_ran = true;
                output.flight_output = app_.step(input.flight_input);

                std::uint64_t completion_tick = input.tick;
                if (input.simulated_flight_completion_delay_ticks > 0U) {
                    const auto delay = static_cast<std::uint64_t>(
                        input.simulated_flight_completion_delay_ticks
                    );
                    if (completion_tick > std::numeric_limits<std::uint64_t>::max() - delay) {
                        completion_tick = std::numeric_limits<std::uint64_t>::max();
                    } else {
                        completion_tick += delay;
                    }
                }

                output.flight_completion = scheduler_.complete(
                    release.task_id,
                    completion_tick
                );
                break;
            }

            case ExecutiveTaskId::HOUSEKEEPING_SNAPSHOT:
                output.housekeeping_ran = true;
                output.housekeeping_completion = scheduler_.complete(
                    release.task_id,
                    input.tick
                );
                break;
        }
    }

    output.scheduler_statistics = scheduler_.statistics();
    return output;
}

Mode FlightSoftwareExecutive::current_mode() const {
    return app_.current_mode();
}

FaultCode FlightSoftwareExecutive::last_fault() const {
    return app_.last_fault();
}

std::vector<RateGroupConfig> FlightSoftwareExecutive::make_scheduler_configuration(
    const FlightSoftwareExecutiveConfig& configuration
) {
    RateGroupConfig flight;
    flight.task_id = static_cast<std::uint16_t>(ExecutiveTaskId::FLIGHT_SOFTWARE_STEP);
    flight.name = "flight_software_step";
    flight.period_ticks = configuration.flight_period_ticks;
    flight.phase_ticks = configuration.flight_phase_ticks;
    flight.relative_deadline_ticks = configuration.flight_deadline_ticks;

    RateGroupConfig housekeeping;
    housekeeping.task_id = static_cast<std::uint16_t>(ExecutiveTaskId::HOUSEKEEPING_SNAPSHOT);
    housekeeping.name = "housekeeping_snapshot";
    housekeeping.period_ticks = configuration.housekeeping_period_ticks;
    housekeeping.phase_ticks = configuration.housekeeping_phase_ticks;
    housekeeping.relative_deadline_ticks = configuration.housekeeping_deadline_ticks;

    return {flight, housekeeping};
}

}  // namespace astra
