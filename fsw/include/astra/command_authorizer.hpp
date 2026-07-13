#pragma once

#include "astra/command_packet.hpp"

#include <cstdint>
#include <string>

namespace astra {

enum class CommandAuthorizationStatus : std::uint8_t {
    AUTHORIZED = 0,
    REJECTED_COMMAND = 1,
    REJECTED_ARGUMENT = 2
};

struct CommandAuthorizationConfig {
    bool allow_noop = true;
    bool allow_set_mode = true;
    bool allow_fault_injection = true;
    bool allow_telemetry_request = true;
    bool allow_fault_clear = true;
    bool allow_test_mode = true;
};

struct CommandAuthorizationResult {
    CommandAuthorizationStatus status = CommandAuthorizationStatus::AUTHORIZED;
    bool authorized = true;
    std::string message = "Command authorized";
};

class CommandAuthorizer {
public:
    explicit CommandAuthorizer(
        CommandAuthorizationConfig configuration = CommandAuthorizationConfig{}
    );

    CommandAuthorizationResult authorize(const CommandPacket& packet) const;

private:
    CommandAuthorizationConfig configuration_;
};

std::string command_authorization_status_to_string(
    CommandAuthorizationStatus status
);

}  // namespace astra
