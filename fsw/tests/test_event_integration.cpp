#include "astra/flight_software_app.hpp"

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

astra::FlightSoftwareStepInput nominal_input(std::uint64_t timestamp_ms) {
    astra::FlightSoftwareStepInput input;
    input.timestamp_ms = timestamp_ms;
    input.cpu_load_percent = 10.0F;
    input.memory_load_percent = 20.0F;
    input.heartbeat_count = static_cast<std::uint32_t>(timestamp_ms);
    input.sensor_age_ms = 0U;
    input.payload_age_ms = 0U;
    input.loop_duration_ms = 10U;
    return input;
}

astra::CommandPacket set_mode(
    astra::Mode mode,
    std::uint32_t sequence,
    std::uint64_t timestamp_ms
) {
    astra::CommandPacket packet;
    packet.sequence_number = sequence;
    packet.timestamp_ms = timestamp_ms;
    packet.command_id = astra::CommandId::SET_MODE;
    packet.argument = static_cast<std::uint32_t>(mode);
    return packet;
}

void test_command_and_mode_events_are_recorded() {
    astra::FlightSoftwareApp app;
    auto input = nominal_input(1000U);
    input.has_command = true;
    input.command = set_mode(astra::Mode::NOMINAL, 1U, 1000U);
    static_cast<void>(app.step(input));

    const auto events = app.event_snapshot();
    expect_true(events.size() == 2U, "accepted mode command records command and mode events");
    expect_true(events.at(0).source == astra::EventSource::COMMAND, "first event is command disposition");
    expect_true(events.at(0).severity == astra::EventSeverity::INFO, "accepted command event is informational");
    expect_true(events.at(0).message.find("status=ACCEPTED") != std::string::npos, "command event records disposition");
    expect_true(events.at(1).source == astra::EventSource::MODE, "second event is mode change");
    expect_true(events.at(1).message == "mode=BOOT->NOMINAL", "mode event records exact transition");
}

void test_rejected_command_is_warning_without_mode_event() {
    astra::FlightSoftwareApp app;
    auto first = nominal_input(1000U);
    first.has_command = true;
    first.command = set_mode(astra::Mode::NOMINAL, 1U, 1000U);
    static_cast<void>(app.step(first));

    auto duplicate = nominal_input(1010U);
    duplicate.has_command = true;
    duplicate.command = set_mode(astra::Mode::SAFE, 1U, 1010U);
    static_cast<void>(app.step(duplicate));

    const auto events = app.event_snapshot();
    expect_true(events.size() == 3U, "duplicate adds one command event only");
    expect_true(events.back().source == astra::EventSource::COMMAND, "duplicate event source is command");
    expect_true(events.back().severity == astra::EventSeverity::WARNING, "rejected command event is warning");
    expect_true(
        events.back().message.find("REJECTED_DUPLICATE_SEQUENCE") != std::string::npos,
        "duplicate event records rejection type"
    );
}

void test_fault_event_records_fdir_response() {
    astra::FlightSoftwareApp app;
    auto first = nominal_input(1000U);
    first.has_command = true;
    first.command = set_mode(astra::Mode::NOMINAL, 1U, 1000U);
    static_cast<void>(app.step(first));

    auto overload = nominal_input(1100U);
    overload.cpu_load_percent = 97.0F;
    static_cast<void>(app.step(overload));

    const auto events = app.event_snapshot();
    expect_true(events.size() == 4U, "CPU fault adds mode and fault events");
    expect_true(events.at(2).source == astra::EventSource::MODE, "degradation records mode event");
    expect_true(events.at(3).source == astra::EventSource::FDIR, "fault change records FDIR event");
    expect_true(events.at(3).severity == astra::EventSeverity::ERROR, "degraded fault event maps to ERROR");
    expect_true(
        events.at(3).message.find("CPU_OVERLOAD") != std::string::npos &&
            events.at(3).message.find("ENTER_DEGRADED_PAYLOAD") != std::string::npos,
        "FDIR event records fault identity and response"
    );
}

void test_unchanged_fault_is_not_logged_repeatedly() {
    astra::FlightSoftwareApp app;
    auto first = nominal_input(1000U);
    first.has_command = true;
    first.command = set_mode(astra::Mode::NOMINAL, 1U, 1000U);
    static_cast<void>(app.step(first));

    auto overload = nominal_input(1100U);
    overload.cpu_load_percent = 97.0F;
    static_cast<void>(app.step(overload));
    const auto first_count = app.event_snapshot().size();

    overload.timestamp_ms = 1200U;
    static_cast<void>(app.step(overload));
    const auto second_count = app.event_snapshot().size();

    expect_true(first_count == second_count, "persistent unchanged fault does not flood event history");
    expect_true(app.dropped_event_count() == 0U, "short event history has no overflow");
}

}  // namespace

int main() {
    std::cout << "Running flight event integration tests..." << std::endl;

    test_command_and_mode_events_are_recorded();
    test_rejected_command_is_warning_without_mode_event();
    test_fault_event_records_fdir_response();
    test_unchanged_fault_is_not_logged_repeatedly();

    if (failures == 0) {
        std::cout << "All flight event integration tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " flight event integration test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
