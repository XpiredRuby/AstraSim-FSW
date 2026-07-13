#include "astra/command_authorizer.hpp"

#include "astra/mode_manager.hpp"

namespace astra {
namespace {

CommandAuthorizationResult authorized() {
    return {};
}

CommandAuthorizationResult rejected(
    CommandAuthorizationStatus status,
    const std::string& message
) {
    CommandAuthorizationResult result;
    result.status = status;
    result.authorized = false;
    result.message = message;
    return result;
}

}  // namespace

CommandAuthorizer::CommandAuthorizer(CommandAuthorizationConfig configuration)
    : configuration_(configuration) {}

CommandAuthorizationResult CommandAuthorizer::authorize(
    const CommandPacket& packet
) const {
    switch (packet.command_id) {
        case CommandId::NOOP:
            return configuration_.allow_noop
                ? authorized()
                : rejected(
                      CommandAuthorizationStatus::REJECTED_COMMAND,
                      "NOOP is disabled by command authorization policy"
                  );

        case CommandId::SET_MODE: {
            if (!configuration_.allow_set_mode) {
                return rejected(
                    CommandAuthorizationStatus::REJECTED_COMMAND,
                    "SET_MODE is disabled by command authorization policy"
                );
            }

            Mode requested_mode = Mode::BOOT;
            if (command_argument_to_mode(packet.argument, requested_mode) &&
                requested_mode == Mode::TEST &&
                !configuration_.allow_test_mode) {
                return rejected(
                    CommandAuthorizationStatus::REJECTED_ARGUMENT,
                    "TEST mode is disabled by command authorization policy"
                );
            }

            return authorized();
        }

        case CommandId::INJECT_FAULT:
            return configuration_.allow_fault_injection
                ? authorized()
                : rejected(
                      CommandAuthorizationStatus::REJECTED_COMMAND,
                      "INJECT_FAULT is disabled by command authorization policy"
                  );

        case CommandId::REQUEST_TELEMETRY:
            return configuration_.allow_telemetry_request
                ? authorized()
                : rejected(
                      CommandAuthorizationStatus::REJECTED_COMMAND,
                      "REQUEST_TELEMETRY is disabled by command authorization policy"
                  );

        case CommandId::CLEAR_FAULT:
            return configuration_.allow_fault_clear
                ? authorized()
                : rejected(
                      CommandAuthorizationStatus::REJECTED_COMMAND,
                      "CLEAR_FAULT is disabled by command authorization policy"
                  );
    }

    return rejected(
        CommandAuthorizationStatus::REJECTED_COMMAND,
        "Unknown command is not authorized"
    );
}

std::string command_authorization_status_to_string(
    CommandAuthorizationStatus status
) {
    switch (status) {
        case CommandAuthorizationStatus::AUTHORIZED:
            return "AUTHORIZED";
        case CommandAuthorizationStatus::REJECTED_COMMAND:
            return "REJECTED_COMMAND";
        case CommandAuthorizationStatus::REJECTED_ARGUMENT:
            return "REJECTED_ARGUMENT";
    }

    return "UNKNOWN_AUTHORIZATION_STATUS";
}

}  // namespace astra
