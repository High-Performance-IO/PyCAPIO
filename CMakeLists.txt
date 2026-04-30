cmake_minimum_required(VERSION 3.15)
project(libcapio)

include(FetchContent)
include(ExternalProject)
include(GNUInstallDirs)

option(CAPIO_LOG "Enable log capabilities within the CAPIO communication queues and libcapio adapter" OFF)

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

FetchContent_Declare(
        capio
        GIT_REPOSITORY https://github.com/High-Performance-IO/capio.git
        GIT_TAG master
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCAPIO_LOG=${CAPIO_LOG}
        -DCAPIO_BUILD_POSIX=OFF
)


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

pybind11_add_module(_pycapio pycapio.cpp)

target_compile_definitions(_pycapio PRIVATE PYCAPIO_BINDINGS)

message(STATUS "CAPIO_PREFIX = ${capio_SOURCE_DIR}")

target_include_directories(_pycapio PRIVATE
        ${capio_SOURCE_DIR}/capio
        ${capio_SOURCE_DIR}/capio/posix
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

##############################
# Libcapio example targets
#############################

#file(GLOB example_sources
#        CONFIGURE_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/*.cpp"
#)

#foreach (example_src ${example_sources})
#    get_filename_component(example_name ${example_src} NAME_WE)
#    add_executable(${example_name} ${example_src})

#    target_include_directories(${example_name} PRIVATE
#            ${capio_SOURCE_DIR}/capio
#            ${capio_SOURCE_DIR}/capio/posix
#    )

#    target_compile_options(
#            ${example_name}
#            PRIVATE
#            $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
#    )

#    target_link_libraries(${example_name} PRIVATE rt)

#    install(
#            TARGETS ${example_name}
#            RUNTIME DESTINATION bin
#    )
#endforeach ()


##############################
# CapioTextIOWrapper.cpp test target
#############################

#add_executable(test_CapioTextIOWrapper tests/CapioTextIOWrapper.cpp)

#target_include_directories(test_CapioTextIOWrapper PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${capio_SOURCE_DIR}/capio
#        ${capio_SOURCE_DIR}/capio/posix
#)

#target_compile_options(
#        test_CapioTextIOWrapper
#        PRIVATE
#        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
#)

#target_link_libraries(test_CapioTextIOWrapper PRIVATE rt)

#install(
#        TARGETS test_CapioTextIOWrapper
#        RUNTIME DESTINATION bin
#)
