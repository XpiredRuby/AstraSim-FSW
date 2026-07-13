#include "astra/configuration_service.hpp"

#include <limits>
#include <utility>

namespace astra {
namespace {

ConfigurationValidationResult invalid(
    const char* field,
    const char* message
) {
    return {false, field, message};
}

ConfigurationValidationResult validate_rate_group(
    const char* prefix,
    std::uint32_t period_ticks,
    std::uint32_t phase_ticks,
    std::uint32_t deadline_ticks
) {
    if (period_ticks == 0U) {
        return invalid(prefix, "period must be greater than zero");
    }
    if (phase_ticks >= period_ticks) {
        return invalid(prefix, "phase must be less than period");
    }
    if (deadline_ticks == 0U) {
        return invalid(prefix, "deadline must be greater than zero");
    }
    if (deadline_ticks > period_ticks) {
        return invalid(prefix, "deadline must not exceed period");
    }
    return {true, "", ""};
}

}  // namespace

ConfigurationService::ConfigurationService(
    SystemConfiguration initial_configuration
)
    : initial_validation_(validate_configuration(initial_configuration)),
      active_configuration_(std::move(initial_configuration)),
      valid_(initial_validation_.valid) {}

bool ConfigurationService::valid() const {
    return valid_;
}

bool ConfigurationService::locked() const {
    return locked_;
}

const ConfigurationValidationResult& ConfigurationService::initial_validation() const {
    return initial_validation_;
}

const SystemConfiguration& ConfigurationService::active() const {
    return active_configuration_;
}

ConfigurationApplyResult ConfigurationService::apply(
    const SystemConfiguration& candidate,
    std::uint64_t expected_active_revision
) {
    ConfigurationApplyResult result;
    result.previous_revision = active_configuration_.revision;
    result.active_revision = active_configuration_.revision;

    if (locked_) {
        result.status = ConfigurationApplyStatus::REJECTED_LOCKED;
        return result;
    }

    if (expected_active_revision != active_configuration_.revision) {
        result.status = ConfigurationApplyStatus::REJECTED_REVISION_CONFLICT;
        return result;
    }

    if (candidate.revision <= active_configuration_.revision) {
        result.status = ConfigurationApplyStatus::REJECTED_NON_MONOTONIC_REVISION;
        return result;
    }

    result.validation = validate_configuration(candidate);
    if (!result.validation.valid) {
        result.status =
            candidate.schema_version != ASTRA_CONFIGURATION_SCHEMA_VERSION
                ? ConfigurationApplyStatus::REJECTED_INVALID_SCHEMA
                : ConfigurationApplyStatus::REJECTED_INVALID_VALUE;
        return result;
    }

    active_configuration_ = candidate;
    valid_ = true;
    result.status = ConfigurationApplyStatus::ACCEPTED;
    result.active_revision = active_configuration_.revision;
    return result;
}

bool ConfigurationService::lock() {
    if (!valid_ || locked_) {
        return false;
    }
    locked_ = true;
    return true;
}

ConfigurationValidationResult validate_configuration(
    const SystemConfiguration& configuration
) {
    if (configuration.schema_version != ASTRA_CONFIGURATION_SCHEMA_VERSION) {
        return invalid("schema_version", "unsupported configuration schema version");
    }
    if (configuration.revision == 0U) {
        return invalid("revision", "revision must be greater than zero");
    }
    if (configuration.executive.app.event_log_capacity < 16U ||
        configuration.executive.app.event_log_capacity > 4096U) {
        return invalid("executive.app.event_log_capacity", "event capacity must be within [16, 4096]");
    }
    if (configuration.executive.app.ground_command_guard.maximum_age_ms ==
        std::numeric_limits<std::uint64_t>::max()) {
        return invalid("executive.app.ground_command_guard.maximum_age_ms", "maximum age must be bounded");
    }
    if (configuration.executive.app.ground_command_guard.maximum_future_skew_ms ==
        std::numeric_limits<std::uint64_t>::max()) {
        return invalid("executive.app.ground_command_guard.maximum_future_skew_ms", "future skew must be bounded");
    }

    const auto flight = validate_rate_group(
        "executive.flight_rate_group",
        configuration.executive.flight_period_ticks,
        configuration.executive.flight_phase_ticks,
        configuration.executive.flight_deadline_ticks
    );
    if (!flight.valid) {
        return flight;
    }

    const auto housekeeping = validate_rate_group(
        "executive.housekeeping_rate_group",
        configuration.executive.housekeeping_period_ticks,
        configuration.executive.housekeeping_phase_ticks,
        configuration.executive.housekeeping_deadline_ticks
    );
    if (!housekeeping.valid) {
        return housekeeping;
    }

    return {true, "", ""};
}

std::string configuration_apply_status_to_string(
    ConfigurationApplyStatus status
) {
    switch (status) {
        case ConfigurationApplyStatus::ACCEPTED:
            return "ACCEPTED";
        case ConfigurationApplyStatus::REJECTED_INVALID_SCHEMA:
            return "REJECTED_INVALID_SCHEMA";
        case ConfigurationApplyStatus::REJECTED_INVALID_VALUE:
            return "REJECTED_INVALID_VALUE";
        case ConfigurationApplyStatus::REJECTED_REVISION_CONFLICT:
            return "REJECTED_REVISION_CONFLICT";
        case ConfigurationApplyStatus::REJECTED_NON_MONOTONIC_REVISION:
            return "REJECTED_NON_MONOTONIC_REVISION";
        case ConfigurationApplyStatus::REJECTED_LOCKED:
            return "REJECTED_LOCKED";
    }

    return "UNKNOWN_CONFIGURATION_STATUS";
}

}  // namespace astra
