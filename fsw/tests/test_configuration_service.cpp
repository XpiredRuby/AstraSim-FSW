#include "astra/configuration_service.hpp"

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

void test_default_configuration_is_valid() {
    astra::ConfigurationService service;

    expect_true(service.valid(), "default configuration service is valid");
    expect_true(service.initial_validation().valid, "default configuration validation passes");
    expect_true(service.active().revision == 1U, "default active revision is one");
    expect_true(!service.locked(), "default configuration is initially unlocked");
}

void test_valid_revision_update_is_atomic() {
    astra::ConfigurationService service;
    auto candidate = service.active();
    candidate.revision = 2U;
    candidate.executive.housekeeping_period_ticks = 20U;
    candidate.executive.housekeeping_deadline_ticks = 2U;

    const auto result = service.apply(candidate, 1U);

    expect_true(result.status == astra::ConfigurationApplyStatus::ACCEPTED, "valid revision update accepted");
    expect_true(result.previous_revision == 1U, "apply result records previous revision");
    expect_true(result.active_revision == 2U, "apply result records active revision");
    expect_true(service.active().revision == 2U, "active configuration advances atomically");
    expect_true(service.active().executive.housekeeping_period_ticks == 20U, "accepted value becomes active");
}

void test_revision_conflict_preserves_active_configuration() {
    astra::ConfigurationService service;
    auto candidate = service.active();
    candidate.revision = 2U;
    candidate.executive.housekeeping_period_ticks = 20U;

    const auto result = service.apply(candidate, 99U);

    expect_true(result.status == astra::ConfigurationApplyStatus::REJECTED_REVISION_CONFLICT, "stale expected revision rejected");
    expect_true(service.active().revision == 1U, "revision conflict preserves active revision");
    expect_true(service.active().executive.housekeeping_period_ticks == 10U, "revision conflict preserves active values");
}

void test_non_monotonic_revision_is_rejected() {
    astra::ConfigurationService service;
    auto candidate = service.active();

    const auto equal = service.apply(candidate, 1U);
    candidate.revision = 0U;
    const auto lower = service.apply(candidate, 1U);

    expect_true(equal.status == astra::ConfigurationApplyStatus::REJECTED_NON_MONOTONIC_REVISION, "equal revision rejected");
    expect_true(lower.status == astra::ConfigurationApplyStatus::REJECTED_NON_MONOTONIC_REVISION, "lower revision rejected");
}

void test_invalid_candidate_is_rejected_without_partial_update() {
    astra::ConfigurationService service;
    auto candidate = service.active();
    candidate.revision = 2U;
    candidate.executive.flight_period_ticks = 0U;
    candidate.executive.housekeeping_period_ticks = 20U;

    const auto result = service.apply(candidate, 1U);

    expect_true(result.status == astra::ConfigurationApplyStatus::REJECTED_INVALID_VALUE, "invalid schedule rejected");
    expect_true(!result.validation.valid, "invalid apply returns validation failure");
    expect_true(result.validation.field == "executive.flight_rate_group", "invalid field identified");
    expect_true(service.active().revision == 1U, "invalid candidate does not advance revision");
    expect_true(service.active().executive.housekeeping_period_ticks == 10U, "invalid candidate has no partial effect");
}

void test_schema_and_bounds_are_validated() {
    astra::SystemConfiguration bad_schema;
    bad_schema.schema_version = 99U;
    astra::ConfigurationService invalid_initial(bad_schema);
    expect_true(!invalid_initial.valid(), "unsupported initial schema invalidates service");
    expect_true(invalid_initial.initial_validation().field == "schema_version", "schema diagnostic identifies field");

    astra::ConfigurationService service;
    auto candidate = service.active();
    candidate.revision = 2U;
    candidate.schema_version = 99U;
    const auto schema_result = service.apply(candidate, 1U);
    expect_true(schema_result.status == astra::ConfigurationApplyStatus::REJECTED_INVALID_SCHEMA, "unsupported candidate schema rejected");

    candidate = service.active();
    candidate.revision = 2U;
    candidate.executive.app.event_log_capacity = 8U;
    const auto capacity_result = service.apply(candidate, 1U);
    expect_true(capacity_result.status == astra::ConfigurationApplyStatus::REJECTED_INVALID_VALUE, "undersized event capacity rejected");

    candidate = service.active();
    candidate.revision = 2U;
    candidate.executive.app.ground_command_guard.maximum_age_ms =
        std::numeric_limits<std::uint64_t>::max();
    const auto unbounded_result = service.apply(candidate, 1U);
    expect_true(unbounded_result.status == astra::ConfigurationApplyStatus::REJECTED_INVALID_VALUE, "unbounded command age rejected");
}

void test_lock_prevents_runtime_changes() {
    astra::ConfigurationService service;
    expect_true(service.lock(), "valid configuration can be locked");
    expect_true(service.locked(), "lock state exposed");
    expect_true(!service.lock(), "second lock request rejected");

    auto candidate = service.active();
    candidate.revision = 2U;
    const auto result = service.apply(candidate, 1U);
    expect_true(result.status == astra::ConfigurationApplyStatus::REJECTED_LOCKED, "locked configuration rejects update");
    expect_true(service.active().revision == 1U, "locked update preserves revision");
}

void test_status_strings_are_stable() {
    expect_true(
        astra::configuration_apply_status_to_string(
            astra::ConfigurationApplyStatus::REJECTED_REVISION_CONFLICT
        ) == "REJECTED_REVISION_CONFLICT",
        "revision conflict string stable"
    );
    expect_true(
        astra::configuration_apply_status_to_string(
            astra::ConfigurationApplyStatus::REJECTED_LOCKED
        ) == "REJECTED_LOCKED",
        "locked string stable"
    );
}

}  // namespace

int main() {
    std::cout << "Running ConfigurationService tests..." << std::endl;

    test_default_configuration_is_valid();
    test_valid_revision_update_is_atomic();
    test_revision_conflict_preserves_active_configuration();
    test_non_monotonic_revision_is_rejected();
    test_invalid_candidate_is_rejected_without_partial_update();
    test_schema_and_bounds_are_validated();
    test_lock_prevents_runtime_changes();
    test_status_strings_are_stable();

    if (failures == 0) {
        std::cout << "All ConfigurationService tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " ConfigurationService test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
