cmake_minimum_required(VERSION 3.20)

project(ADAM
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "ADAM - Modern tournament-grade UCI Chess Engine written in C++"
)

# -----------------------------------------------------------------------------
# Default Build Type to Release
# -----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Release, Debug, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# -----------------------------------------------------------------------------
# Language Standards
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Project Options
# -----------------------------------------------------------------------------
option(ENABLE_NATIVE_ARCH "Generate architecture-native instructions (-march=native / /arch:AVX2)" ON)
option(ENABLE_LTO "Enable Link-Time Optimization (IPO/LTO)" ON)
option(BUILD_TESTS "Build ADAM unit test suite" ON)
option(BUILD_TOOLS "Build developer utility tools (e.g., magicCreate)" OFF)
option(COPY_TO_PROJECT_ROOT "Automatically copy ADAM executable to project root for test scripts" ON)

# -----------------------------------------------------------------------------
# Interprocedural Optimization (LTO)
# -----------------------------------------------------------------------------
if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "Link-Time Optimization (LTO) enabled")
    else()
        message(STATUS "Link-Time Optimization not supported: ${ipo_error}")
    endif()
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# Compiler Flags & Optimizations
# -----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
    )

    if(ENABLE_NATIVE_ARCH)
        add_compile_options(-march=native)
    endif()

elseif(MSVC)
    add_compile_options(
        /W3
        /permissive-
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ot>
        $<$<CONFIG:Release>:/DNDEBUG>
    )

    if(ENABLE_NATIVE_ARCH)
        add_compile_options(/arch:AVX2)
    endif()
endif()

# -----------------------------------------------------------------------------
# Core Engine Library Target
# -----------------------------------------------------------------------------
set(ADAM_CORE_SOURCES
    Engine/board/board.cpp
    Engine/moves/move.cpp
    Engine/moves/movegen.cpp
    Engine/attack/attacks.cpp
    Engine/attack/magic.cpp
    Engine/attack/magic_instance.cpp
    Engine/evaluation/eval.cpp
    Engine/perft/perft.cpp
    Engine/search/search.cpp
    Engine/uci/uci.cpp
    Engine/utils/bitboard_utilities.cpp
    Engine/utils/tools.cpp
    Engine/hash/zobrist.cpp
    Engine/hash/tt.cpp
    Engine/hash/kpk.cpp
    Engine/syzygy/syzygy.cpp
    Engine/syzygy/tbprobe.c
)

add_library(adam_core STATIC ${ADAM_CORE_SOURCES})

target_include_directories(adam_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/syzygy>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(adam_core PUBLIC Threads::Threads)

# -----------------------------------------------------------------------------
# Main Executable Target
# -----------------------------------------------------------------------------
add_executable(ADAM Engine/main.cpp)
target_link_libraries(ADAM PRIVATE adam_core)
set_target_properties(ADAM PROPERTIES OUTPUT_NAME "ADAM")

# Automatically sync binary to project root for run_test.bat & run_test.ps1
if(COPY_TO_PROJECT_ROOT AND NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_custom_command(TARGET ADAM POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ADAM>
            ${CMAKE_CURRENT_SOURCE_DIR}/$<TARGET_FILE_NAME:ADAM>
        COMMENT "Copying ADAM binary to project root: ${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# -----------------------------------------------------------------------------
# Unit Tests (CTest Integration)
# -----------------------------------------------------------------------------
if(BUILD_TESTS)
    enable_testing()

    function(adam_add_test test_target test_source)
        add_executable(${test_target} ${test_source})
        target_link_libraries(${test_target} PRIVATE adam_core)
        add_test(NAME ${test_target} COMMAND ${test_target})
    endfunction()

    adam_add_test(test_makemove   Engine/test/makemove.cpp)
    adam_add_test(test_movegen    Engine/test/movegen.cpp)
    adam_add_test(test_movetester Engine/test/moveTester.cpp)
    adam_add_test(test_undomove   Engine/test/undoMoveTest.cpp)
    adam_add_test(test_attacks    Engine/test/test_attacks.cpp)
    adam_add_test(test_magic      Engine/test/test_magic.cpp)
    adam_add_test(test_eval       Engine/test/test_eval.cpp)
    adam_add_test(test_run_eval   Engine/test/run_eval.cpp)
endif()

# -----------------------------------------------------------------------------
# Auxiliary Developer Tools
# -----------------------------------------------------------------------------
if(BUILD_TOOLS)
    add_executable(magicCreate
        Engine/attack/magicCreate.cpp
        Engine/attack/magicGen.cpp
    )
    target_link_libraries(magicCreate PRIVATE adam_core)
endif()

# -----------------------------------------------------------------------------
# Installation Rules
# -----------------------------------------------------------------------------
install(TARGETS ADAM RUNTIME DESTINATION bin)
