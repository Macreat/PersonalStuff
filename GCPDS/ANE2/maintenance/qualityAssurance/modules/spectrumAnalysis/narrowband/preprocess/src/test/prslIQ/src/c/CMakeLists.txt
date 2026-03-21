cmake_minimum_required(VERSION 3.16)
project(iq_method3_bench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(IQBENCH_NATIVE_OPT "Enable native CPU optimizations for local benchmarking" OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

add_executable(iq_method3_bench
    src/main.c
    src/fs_utils.c
    src/sigmf_io.c
    src/method3.c
    src/welch.c
    src/profiler.c
)

target_include_directories(iq_method3_bench PRIVATE include)

if(MSVC)
    target_compile_options(iq_method3_bench PRIVATE /W4)
    target_compile_options(iq_method3_bench PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(iq_method3_bench PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(iq_method3_bench PRIVATE $<$<CONFIG:Release>:-O3>)
    if(IQBENCH_NATIVE_OPT)
        target_compile_options(iq_method3_bench PRIVATE -march=native -mtune=native)
    endif()
endif()

if(IPO_SUPPORTED)
    set_property(TARGET iq_method3_bench PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

if(WIN32)
    target_link_libraries(iq_method3_bench PRIVATE psapi)
else()
    target_link_libraries(iq_method3_bench PRIVATE m)
endif()
