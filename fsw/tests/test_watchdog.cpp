#include "astra/watchdog.hpp"

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

astra::WatchdogConfig test_config() {
    astra::WatchdogConfig config;
    config.warning_ms = 400;
    config.timeout_ms = 500;
    config.loop_deadline_ms = 250;
    config.max_missed_deadlines = 3;
    return config;
}

void test_kick_updates_last_kick_time() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);

    expect_true(watchdog.last_kick_ms() == 1000, "kick updates last kick time");
}

void test_nominal_evaluation_is_ok() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);
    const auto report = watchdog.evaluate(1100, 20);

    expect_true(report.status == astra::WatchdogStatus::OK, "nominal watchdog reports OK");
    expect_true(report.fault == astra::FaultCode::NONE, "nominal watchdog has no fault");
    expect_true(report.alive, "nominal watchdog remains alive");
    expect_true(report.loop_timing_ok, "nominal loop timing is OK");
    expect_true(report.elapsed_since_kick_ms == 100, "elapsed time is computed");
}

void test_warning_before_timeout() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);
    const auto report = watchdog.evaluate(1450, 20);

    expect_true(report.status == astra::WatchdogStatus::WARNING, "warning threshold produces WARNING");
    expect_true(report.fault == astra::FaultCode::NONE, "warning does not produce fault");
    expect_true(report.alive, "warning watchdog remains alive");
}

void test_timeout_expires_watchdog() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);
    const auto report = watchdog.evaluate(1600, 20);

    expect_true(report.status == astra::WatchdogStatus::EXPIRED, "timeout produces EXPIRED");
    expect_true(
        report.fault == astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        "timeout maps to WATCHDOG_DEADLINE_MISS"
    );
    expect_true(!report.alive, "timeout marks watchdog not alive");
}

void test_loop_deadline_miss_warning() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);
    const auto report = watchdog.evaluate(1100, 300);

    expect_true(report.status == astra::WatchdogStatus::WARNING, "loop deadline miss reports WARNING");
    expect_true(!report.loop_timing_ok, "loop deadline miss marks timing bad");
    expect_true(report.missed_deadline_count == 1, "missed deadline count increments");
}

void test_repeated_deadline_misses_expire_watchdog() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);

    watchdog.evaluate(1100, 300);
    watchdog.evaluate(1200, 300);
    const auto report = watchdog.evaluate(1300, 300);

    expect_true(report.status == astra::WatchdogStatus::EXPIRED, "repeated misses expire watchdog");
    expect_true(
        report.fault == astra::FaultCode::WATCHDOG_DEADLINE_MISS,
        "repeated misses map to WATCHDOG_DEADLINE_MISS"
    );
    expect_true(report.missed_deadline_count == 3, "missed deadline count reaches limit");
}

void test_reset_clears_missed_deadlines() {
    astra::Watchdog watchdog(test_config());

    watchdog.kick(1000);
    watchdog.evaluate(1100, 300);
    watchdog.evaluate(1200, 300);

    watchdog.reset(2000);
    const auto report = watchdog.evaluate(2050, 20);

    expect_true(watchdog.last_kick_ms() == 2000, "reset updates last kick time");
    expect_true(watchdog.missed_deadline_count() == 0, "reset clears missed deadline count");
    expect_true(report.status == astra::WatchdogStatus::OK, "reset returns watchdog to OK");
}

void test_status_strings() {
    expect_true(
        astra::watchdog_status_to_string(astra::WatchdogStatus::EXPIRED) == "EXPIRED",
        "watchdog status string conversion works"
    );
}

}  // namespace

int main() {
    std::cout << "Running Watchdog tests..." << std::endl;

    test_kick_updates_last_kick_time();
    test_nominal_evaluation_is_ok();
    test_warning_before_timeout();
    test_timeout_expires_watchdog();
    test_loop_deadline_miss_warning();
    test_repeated_deadline_misses_expire_watchdog();
    test_reset_clears_missed_deadlines();
    test_status_strings();

    if (failures == 0) {
        std::cout << "All Watchdog tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " Watchdog test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
