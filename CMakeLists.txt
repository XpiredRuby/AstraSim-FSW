cmake_minimum_required(VERSION 3.16)

project(AstraSimFSW VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ASTRA_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(ASTRA_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ASTRA_ENABLE_COVERAGE "Enable GCC/Clang coverage instrumentation" OFF)
option(ASTRA_ENABLE_CLANG_TIDY "Run clang-tidy as part of the build" OFF)
option(ASTRA_BUILD_FUZZERS "Build Clang libFuzzer targets" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)

    if(ASTRA_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()

    if(ASTRA_ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()

    if(ASTRA_ENABLE_COVERAGE)
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
elseif(MSVC)
    add_compile_options(/W4)

    if(ASTRA_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
endif()

if(ASTRA_ENABLE_CLANG_TIDY)
    find_program(ASTRA_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY
        "${ASTRA_CLANG_TIDY_EXECUTABLE};--warnings-as-errors=*"
    )
endif()

add_library(astra_fsw_core
    fsw/src/mode_manager.cpp
    fsw/src/fdir_manager.cpp
    fsw/src/event_logger.cpp
    fsw/src/telemetry_packet.cpp
    fsw/src/udp_telemetry_sender.cpp
    fsw/src/command_packet.cpp
    fsw/src/command_processor.cpp
    fsw/src/udp_command_receiver.cpp
    fsw/src/flight_software_app.cpp
    fsw/src/health_monitor.cpp
    fsw/src/watchdog.cpp
    fsw/src/rate_group_scheduler.cpp
    fsw/src/flight_software_executive.cpp
)

target_include_directories(astra_fsw_core PUBLIC
    fsw/include
)

add_executable(astra_fsw
    fsw/src/main.cpp
)

target_link_libraries(astra_fsw PRIVATE
    astra_fsw_core
)

add_executable(astra_fsw_telemetry_demo
    fsw/src/telemetry_demo.cpp
)

target_link_libraries(astra_fsw_telemetry_demo PRIVATE
    astra_fsw_core
)

add_executable(astra_fsw_command_telemetry_demo
    fsw/src/command_telemetry_demo.cpp
)

target_link_libraries(astra_fsw_command_telemetry_demo PRIVATE
    astra_fsw_core
)

add_executable(astra_fsw_tests
    fsw/tests/test_mode_manager.cpp
)

target_link_libraries(astra_fsw_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_telemetry_tests
    fsw/tests/test_telemetry_packet.cpp
)

target_link_libraries(astra_telemetry_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_udp_telemetry_tests
    fsw/tests/test_udp_telemetry_sender.cpp
)

target_link_libraries(astra_udp_telemetry_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_command_tests
    fsw/tests/test_command_packet.cpp
)

target_link_libraries(astra_command_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_command_processor_tests
    fsw/tests/test_command_processor.cpp
)

target_link_libraries(astra_command_processor_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_udp_command_receiver_tests
    fsw/tests/test_udp_command_receiver.cpp
)

target_link_libraries(astra_udp_command_receiver_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_flight_software_app_tests
    fsw/tests/test_flight_software_app.cpp
)

target_link_libraries(astra_flight_software_app_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_health_monitor_tests
    fsw/tests/test_health_monitor.cpp
)

target_link_libraries(astra_health_monitor_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_watchdog_tests
    fsw/tests/test_watchdog.cpp
)

target_link_libraries(astra_watchdog_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_rate_group_scheduler_tests
    fsw/tests/test_rate_group_scheduler.cpp
)

target_link_libraries(astra_rate_group_scheduler_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_extended_mode_tests
    fsw/tests/test_extended_modes.cpp
)

target_link_libraries(astra_extended_mode_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_fdir_manager_tests
    fsw/tests/test_fdir_manager.cpp
)

target_link_libraries(astra_fdir_manager_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_simultaneous_fault_tests
    fsw/tests/test_simultaneous_faults.cpp
)

target_link_libraries(astra_simultaneous_fault_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_flight_software_executive_tests
    fsw/tests/test_flight_software_executive.cpp
)

target_link_libraries(astra_flight_software_executive_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_event_logger_tests
    fsw/tests/test_event_logger.cpp
)

target_link_libraries(astra_event_logger_tests PRIVATE
    astra_fsw_core
)

add_executable(astra_event_integration_tests
    fsw/tests/test_event_integration.cpp
)

target_link_libraries(astra_event_integration_tests PRIVATE
    astra_fsw_core
)

if(ASTRA_BUILD_FUZZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "ASTRA_BUILD_FUZZERS requires Clang with matching libFuzzer support")
    endif()

    target_compile_options(astra_fsw_core PRIVATE
        -fsanitize=fuzzer-no-link,address,undefined
        -fno-omit-frame-pointer
    )

    add_executable(astra_command_packet_fuzzer
        fsw/fuzz/fuzz_command_packet.cpp
    )

    target_link_libraries(astra_command_packet_fuzzer PRIVATE
        astra_fsw_core
    )

    target_compile_options(astra_command_packet_fuzzer PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer
    )

    target_link_options(astra_command_packet_fuzzer PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer
    )
endif()

enable_testing()
add_test(NAME mode_manager_tests COMMAND astra_fsw_tests)
add_test(NAME telemetry_packet_tests COMMAND astra_telemetry_tests)
add_test(NAME udp_telemetry_sender_tests COMMAND astra_udp_telemetry_tests)
add_test(NAME command_packet_tests COMMAND astra_command_tests)
add_test(NAME command_processor_tests COMMAND astra_command_processor_tests)
add_test(NAME udp_command_receiver_tests COMMAND astra_udp_command_receiver_tests)
add_test(NAME flight_software_app_tests COMMAND astra_flight_software_app_tests)
add_test(NAME health_monitor_tests COMMAND astra_health_monitor_tests)
add_test(NAME watchdog_tests COMMAND astra_watchdog_tests)
add_test(NAME rate_group_scheduler_tests COMMAND astra_rate_group_scheduler_tests)
add_test(NAME extended_mode_tests COMMAND astra_extended_mode_tests)
add_test(NAME fdir_manager_tests COMMAND astra_fdir_manager_tests)
add_test(NAME simultaneous_fault_tests COMMAND astra_simultaneous_fault_tests)
add_test(NAME flight_software_executive_tests COMMAND astra_flight_software_executive_tests)
add_test(NAME event_logger_tests COMMAND astra_event_logger_tests)
add_test(NAME event_integration_tests COMMAND astra_event_integration_tests)
