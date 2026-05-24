cmake_minimum_required(VERSION 3.10)
project(liquid_sdr_bench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Optimization flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wextra -O3 -march=native)
endif()

# Find Liquid-DSP
find_path(LIQUID_INCLUDE_DIR NAMES liquid/liquid.h)
find_library(LIQUID_LIBRARY NAMES liquid)

if(NOT LIQUID_INCLUDE_DIR OR NOT LIQUID_LIBRARY)
    message(FATAL_ERROR "Liquid-DSP not found. Please install it first.")
endif()

# Include directories
include_directories(include)
include_directories(${LIQUID_INCLUDE_DIR})

# Sources
set(SOURCES
    src/main.c
    src/liquid_method3.c
    src/sigmf_io_liquid.c
    src/welch_liquid.c
    src/fs_utils.c
    src/profiler.c
)

# Executable
add_executable(liquid_sdr_bench ${SOURCES})

# Linking
target_link_libraries(liquid_sdr_bench ${LIQUID_LIBRARY} m)

# Installation/Output settings
set_target_properties(liquid_sdr_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
