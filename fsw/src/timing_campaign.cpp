#include "astra/flight_software_executive.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    std::uint64_t ticks = 100000U;
    std::uint64_t mission_step_ms = 10U;
    std::uint64_t overload_every = 0U;
    std::string output_path;
};

std::uint64_t parse_u64(const char* value, const char* option_name) {
    try {
        std::size_t consumed = 0;
        const std::string text(value);
        const auto parsed = std::stoull(text, &consumed, 10);
        if (consumed != text.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return parsed;
    } catch (const std::exception& error) {
        throw std::invalid_argument(
            std::string(option_name) + " requires an unsigned integer: " + error.what()
        );
    }
}

Options parse_options(int argc, char** argv) {
    Options options;

    for (int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if (argument == "--ticks" && index + 1 < argc) {
            options.ticks = parse_u64(argv[++index], "--ticks");
        } else if (argument == "--mission-step-ms" && index + 1 < argc) {
            options.mission_step_ms = parse_u64(argv[++index], "--mission-step-ms");
        } else if (argument == "--overload-every" && index + 1 < argc) {
            options.overload_every = parse_u64(argv[++index], "--overload-every");
        } else if (argument == "--output" && index + 1 < argc) {
            options.output_path = argv[++index];
        } else if (argument == "--help") {
            std::cout
                << "Usage: astra_timing_campaign [--ticks N] [--mission-step-ms N] "
                   "[--overload-every N] [--output PATH]\n";
            std::exit(EXIT_SUCCESS);
        } else {
            throw std::invalid_argument("unknown or incomplete option: " + argument);
        }
    }

    if (options.ticks == 0U) {
        throw std::invalid_argument("--ticks must be greater than zero");
    }
    if (options.mission_step_ms == 0U) {
        throw std::invalid_argument("--mission-step-ms must be greater than zero");
    }
    if (options.ticks > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        throw std::invalid_argument("--ticks exceeds addressable sample count");
    }

    return options;
}

std::uint64_t percentile_ns(
    const std::vector<std::uint64_t>& sorted_samples,
    double percentile
) {
    if (sorted_samples.empty()) {
        return 0U;
    }

    const double scaled = percentile * static_cast<double>(sorted_samples.size() - 1U);
    const auto index = static_cast<std::size_t>(scaled + 0.5);
    return sorted_samples.at(index);
}

std::string json_escape(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (const char character : input) {
        switch (character) {
            case '\\': output += "\\\\"; break;
            case '"': output += "\\\""; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += character; break;
        }
    }
    return output;
}

std::string build_json(
    const Options& options,
    const std::vector<std::uint64_t>& samples,
    std::uint64_t total_elapsed_ns,
    std::uint64_t scheduler_deadline_misses,
    astra::Mode final_mode,
    astra::FaultCode final_fault
) {
    std::vector<std::uint64_t> sorted_samples = samples;
    std::sort(sorted_samples.begin(), sorted_samples.end());

    const auto sum = std::accumulate(
        samples.begin(),
        samples.end(),
        static_cast<long double>(0.0L)
    );
    const long double mean = sum / static_cast<long double>(samples.size());

    std::ostringstream output;
    output << std::fixed << std::setprecision(3);
    output << "{\n";
    output << "  \"schema\": \"astra-os.host-timing.v1\",\n";
    output << "  \"claim_boundary\": \"Host faster-than-real-time execution evidence only; not a real-time, HIL, or flight qualification claim.\",\n";
    output << "  \"ticks\": " << options.ticks << ",\n";
    output << "  \"mission_step_ms\": " << options.mission_step_ms << ",\n";
    output << "  \"overload_every\": " << options.overload_every << ",\n";
    output << "  \"total_elapsed_ns\": " << total_elapsed_ns << ",\n";
    output << "  \"mean_step_ns\": " << static_cast<double>(mean) << ",\n";
    output << "  \"min_step_ns\": " << sorted_samples.front() << ",\n";
    output << "  \"p50_step_ns\": " << percentile_ns(sorted_samples, 0.50) << ",\n";
    output << "  \"p95_step_ns\": " << percentile_ns(sorted_samples, 0.95) << ",\n";
    output << "  \"p99_step_ns\": " << percentile_ns(sorted_samples, 0.99) << ",\n";
    output << "  \"max_step_ns\": " << sorted_samples.back() << ",\n";
    output << "  \"scheduler_deadline_misses\": " << scheduler_deadline_misses << ",\n";
    output << "  \"final_mode\": \"" << json_escape(astra::mode_to_string(final_mode)) << "\",\n";
    output << "  \"final_fault\": \"" << json_escape(astra::fault_to_string(final_fault)) << "\"\n";
    output << "}\n";
    return output.str();
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    try {
        options = parse_options(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    astra::FlightSoftwareExecutive executive;
    if (!executive.valid()) {
        std::cerr << "ERROR: invalid executive: " << executive.validation_error() << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::uint64_t> samples;
    samples.reserve(static_cast<std::size_t>(options.ticks));
    std::uint64_t deadline_misses = 0U;

    const auto campaign_start = std::chrono::steady_clock::now();

    for (std::uint64_t tick = 0U; tick < options.ticks; ++tick) {
        astra::FlightSoftwareExecutiveInput input;
        input.tick = tick;
        input.flight_input.timestamp_ms = tick * options.mission_step_ms;
        input.flight_input.cpu_load_percent = 25.0F;
        input.flight_input.memory_load_percent = 30.0F;
        input.flight_input.heartbeat_count = static_cast<std::uint32_t>(tick);
        input.flight_input.sensor_age_ms = 0U;
        input.flight_input.payload_age_ms = 0U;
        input.flight_input.loop_duration_ms = 1U;

        if (tick == 0U) {
            input.flight_input.has_command = true;
            input.flight_input.command.sequence_number = 1U;
            input.flight_input.command.timestamp_ms = 0U;
            input.flight_input.command.command_id = astra::CommandId::SET_MODE;
            input.flight_input.command.argument = static_cast<std::uint32_t>(astra::Mode::NOMINAL);
        }

        if (options.overload_every > 0U && tick > 0U &&
            tick % options.overload_every == 0U) {
            input.simulated_flight_completion_delay_ticks = 2U;
        }

        const auto step_start = std::chrono::steady_clock::now();
        const auto result = executive.step(input);
        const auto step_finish = std::chrono::steady_clock::now();

        if (result.scheduler_status != astra::SchedulerStatus::OK) {
            std::cerr << "ERROR: scheduler failed at tick " << tick << " status="
                      << astra::scheduler_status_to_string(result.scheduler_status)
                      << std::endl;
            return EXIT_FAILURE;
        }

        samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                step_finish - step_start
            ).count()
        ));

        if (!result.scheduler_statistics.empty()) {
            deadline_misses = result.scheduler_statistics.front().deadline_miss_count;
        }
    }

    const auto campaign_finish = std::chrono::steady_clock::now();
    const auto elapsed_ns = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            campaign_finish - campaign_start
        ).count()
    );

    const auto json = build_json(
        options,
        samples,
        elapsed_ns,
        deadline_misses,
        executive.current_mode(),
        executive.last_fault()
    );

    if (options.output_path.empty()) {
        std::cout << json;
    } else {
        std::ofstream output(options.output_path);
        if (!output) {
            std::cerr << "ERROR: cannot open output path: " << options.output_path << std::endl;
            return EXIT_FAILURE;
        }
        output << json;
        std::cout << "Wrote timing evidence: " << options.output_path << std::endl;
    }

    return EXIT_SUCCESS;
}
