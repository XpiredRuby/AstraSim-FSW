cmake_minimum_required(VERSION 3.16)

project(AstraSimFSW VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(astra_fsw_core
    fsw/src/mode_manager.cpp
    fsw/src/telemetry_packet.cpp
    fsw/src/udp_telemetry_sender.cpp
    fsw/src/command_packet.cpp
    fsw/src/command_processor.cpp
    fsw/src/udp_command_receiver.cpp
    fsw/src/flight_software_app.cpp
    fsw/src/health_monitor.cpp
    fsw/src/watchdog.cpp
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
