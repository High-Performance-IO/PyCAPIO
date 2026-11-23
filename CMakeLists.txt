cmake_minimum_required(VERSION 3.15)
project(PyCAPIO
        LANGUAGES CXX
        DESCRIPTION "Native integration of the CAPIO methodology within the python interpreter"
        VERSION 0.0.2
)

include(FetchContent)
include(ExternalProject)
include(GNUInstallDirs)

option(CAPIO_LOG "Enable log capabilities within the CAPIO communication queues and libcapio adapter" OFF)

set(CAPIO_RELEASE_TAG "master" CACHE STRING "Default CAPIO tag used to compile PyCAPIO.")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling CAPIO Logger")
    add_compile_definitions(CAPIO_LOG)
endif ()

message(STATUS "Using CAPIO on tag ${CAPIO_RELEASE_TAG}")

FetchContent_Declare(
        capio
        GIT_REPOSITORY https://github.com/High-Performance-IO/capio.git
        GIT_TAG ${CAPIO_RELEASE_TAG}
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

set(CAPIO_LOG ${CAPIO_LOG} CACHE BOOL "" FORCE)
set(CAPIO_BUILD_POSIX False CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(pybind11 capio)

if (SKBUILD_SCRIPTS_DIR)
    set(CAPIO_INSTALL_DIR ${SKBUILD_SCRIPTS_DIR})
else ()
    set(CAPIO_INSTALL_DIR bin)
endif ()

install(
        TARGETS capio_server
        RUNTIME DESTINATION ${CAPIO_INSTALL_DIR}
)
########################
# PYBIND11 TARGET
########################

file(GLOB PYCAPIO_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

pybind11_add_module(_pycapio ${PYCAPIO_SOURCES})

# Older CAPIO releases (before the CALF logger refactor, CAPIO#239) kept server_println.hpp in the
# server tree only, while the posix headers included it: copy it over for those releases only.
set(_capio_server_println "${capio_SOURCE_DIR}/capio/server/include/utils/server_println.hpp")
if (EXISTS "${_capio_server_println}")
    file(COPY "${_capio_server_println}" DESTINATION "${capio_SOURCE_DIR}/capio/posix/utils/")
    message(WARNING "WARN: patch for server_println.hpp copied to posix has been applied!")
else ()
    # Newer CAPIO releases log through CALF, and their posix headers expect __CAPIO_POSIX to be
    # defined (logger selection, syscall_intercept primitives). Build the bindings like CAPIO
    # builds its posix library, with include/compat standing in for syscall_intercept, which
    # PyCAPIO does not need since it calls the handlers directly instead of hooking syscalls.
    target_compile_definitions(_pycapio PRIVATE __CAPIO_POSIX)
    target_include_directories(_pycapio PRIVATE ${CMAKE_SOURCE_DIR}/include/compat)

    set(_pycapio_log_enabled FALSE)
    if (CAPIO_LOG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_pycapio_log_enabled TRUE)
    endif ()
    calf_enable_log(_pycapio ${_pycapio_log_enabled})
    calf_set_component(_pycapio "pycapio")
    calf_set_default_log_dir(_pycapio "./capio_logs")
endif ()

target_compile_definitions(_pycapio PRIVATE PYCAPIO_BINDINGS)
target_compile_definitions(_pycapio PRIVATE CAPIO_VERSION="${CMAKE_PROJECT_VERSION}")

message(STATUS "CAPIO_PREFIX = ${capio_SOURCE_DIR}")

target_include_directories(_pycapio PRIVATE
        ${capio_SOURCE_DIR}/capio
        ${capio_SOURCE_DIR}/capio/posix
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(
        _pycapio
        PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

install(
        TARGETS _pycapio
        LIBRARY DESTINATION pycapio
)