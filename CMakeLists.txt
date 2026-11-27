cmake_minimum_required(VERSION 3.16)
project(rvc_controller C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RVC_COVERAGE "Instrument production C code for statement/branch/MC-DC coverage" OFF)

add_compile_options(-Wall -Wextra)

# ---- production code (C11) ----
add_library(rvc_core STATIC
    src/sensor_store.c
    src/capture_sensor_values.c
    src/generate_tick.c
    src/actuator.c
    src/controller.c
    src/rvc_main.c
)
target_include_directories(rvc_core PUBLIC include)
target_compile_options(rvc_core PRIVATE -Wpedantic)

add_executable(rvc src/main.c)
target_link_libraries(rvc PRIVATE rvc_core)

if(RVC_COVERAGE)
    # -fcondition-coverage (GCC 14+) records MC/DC data for gcov.
    target_compile_options(rvc_core PRIVATE -O0 -g --coverage -fcondition-coverage)
    target_link_options(rvc_core PUBLIC --coverage)
    target_compile_options(rvc PRIVATE -O0 -g --coverage -fcondition-coverage)
    target_link_options(rvc PRIVATE --coverage)
endif()

# ---- tests (GoogleTest, C++) ----
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

function(rvc_test_suite name)
    file(GLOB srcs CONFIGURE_DEPENDS ${ARGN})
    if(srcs)
        add_executable(${name} ${srcs})
        target_include_directories(${name} PRIVATE tests/support)
        target_link_libraries(${name} PRIVATE rvc_core GTest::gtest GTest::gtest_main)
        gtest_discover_tests(${name} TEST_PREFIX "${name}." DISCOVERY_TIMEOUT 30)
    endif()
endfunction()

rvc_test_suite(unit_functional ${CMAKE_SOURCE_DIR}/tests/unit/functional/*.cpp)
rvc_test_suite(unit_structural ${CMAKE_SOURCE_DIR}/tests/unit/structural/*.cpp)
rvc_test_suite(system_test     ${CMAKE_SOURCE_DIR}/tests/system/*.cpp)
if(TARGET system_test)
    add_dependencies(system_test rvc)
    target_compile_definitions(system_test PRIVATE RVC_BIN="$<TARGET_FILE:rvc>")
endif()
