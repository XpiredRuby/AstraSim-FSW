#pragma once

#include "astra/flight_software_executive.hpp"

#include <cstdint>
#include <string>

namespace astra {

constexpr std::uint32_t ASTRA_CONFIGURATION_SCHEMA_VERSION = 1U;

enum class ConfigurationApplyStatus : std::uint8_t {
    ACCEPTED = 0,
    REJECTED_INVALID_SCHEMA = 1,
    REJECTED_INVALID_VALUE = 2,
    REJECTED_REVISION_CONFLICT = 3,
    REJECTED_NON_MONOTONIC_REVISION = 4,
    REJECTED_LOCKED = 5
};

struct SystemConfiguration {
    std::uint32_t schema_version = ASTRA_CONFIGURATION_SCHEMA_VERSION;
    std::uint64_t revision = 1U;
    FlightSoftwareExecutiveConfig executive;
};

struct ConfigurationValidationResult {
    bool valid = false;
    std::string field;
    std::string message;
};

struct ConfigurationApplyResult {
    ConfigurationApplyStatus status = ConfigurationApplyStatus::REJECTED_INVALID_VALUE;
    std::uint64_t previous_revision = 0U;
    std::uint64_t active_revision = 0U;
    ConfigurationValidationResult validation;
};

class ConfigurationService {
public:
    explicit ConfigurationService(
        SystemConfiguration initial_configuration = SystemConfiguration{}
    );

    bool valid() const;
    bool locked() const;
    const ConfigurationValidationResult& initial_validation() const;
    const SystemConfiguration& active() const;

    ConfigurationApplyResult apply(
        const SystemConfiguration& candidate,
        std::uint64_t expected_active_revision
    );

    bool lock();

private:
    ConfigurationValidationResult initial_validation_;
    SystemConfiguration active_configuration_;
    bool valid_ = false;
    bool locked_ = false;
};

ConfigurationValidationResult validate_configuration(
    const SystemConfiguration& configuration
);

std::string configuration_apply_status_to_string(
    ConfigurationApplyStatus status
);

}  // namespace astra
