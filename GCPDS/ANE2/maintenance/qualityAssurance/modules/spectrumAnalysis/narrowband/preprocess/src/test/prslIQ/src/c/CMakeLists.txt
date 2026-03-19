cmake_minimum_required(VERSION 3.16)
project(iq_method3_bench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3)
endif()

add_executable(iq_method3_bench
    src/main.c
    src/fs_utils.c
    src/sigmf_io.c
    src/method3.c
    src/welch.c
    src/profiler.c
)

target_include_directories(iq_method3_bench PRIVATE include)

if(WIN32)
    target_link_libraries(iq_method3_bench PRIVATE psapi)
else()
    target_link_libraries(iq_method3_bench PRIVATE m)
endif()
