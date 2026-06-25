cmake_minimum_required(VERSION 3.16)

project(AstraSimFSW VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(astra_fsw_core
    fsw/src/mode_manager.cpp
    fsw/src/telemetry_packet.cpp
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

enable_testing()
add_test(NAME mode_manager_tests COMMAND astra_fsw_tests)
add_test(NAME telemetry_packet_tests COMMAND astra_telemetry_tests)
