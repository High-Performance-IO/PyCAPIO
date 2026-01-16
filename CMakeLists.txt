cmake_minimum_required(VERSION 3.15)
project(libcapio)

include(FetchContent)
include(ExternalProject)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CAPIO_PREFIX ${CMAKE_BINARY_DIR}/external/capio)
set(CAPIO_INSTALL_DIR ${CAPIO_PREFIX}/install)
set(CAPIO_INCLUDE_DIR ${CAPIO_INSTALL_DIR}/include)

FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.1
)
FetchContent_MakeAvailable(pybind11)

file(MAKE_DIRECTORY ${CAPIO_INCLUDE_DIR})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling CAPIO Logger")
    add_compile_definitions(CAPIO_LOG)
endif ()

ExternalProject_Add(
        capio
        PREFIX ${CAPIO_PREFIX}
        GIT_REPOSITORY https://github.com/High-Performance-IO/capio.git
        GIT_TAG        streaming-bugfixes

        # Patch to remove specific subdirectories
        PATCH_COMMAND sed -i "/add_subdirectory(capio\\/posix)/d" <SOURCE_DIR>/CMakeLists.txt
        COMMAND sed -i "/add_subdirectory(capio\\/server)/d" <SOURCE_DIR>/CMakeLists.txt

        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CAPIO_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON

        INSTALL_COMMAND ""
)

add_library(capio_headers INTERFACE)
target_include_directories(capio_headers INTERFACE
        ${CAPIO_PREFIX}/src/capio/capio
        ${CAPIO_PREFIX}/src/capio/capio/posix
)
########################
# PYBIND11 TARGET
########################

pybind11_add_module(
        _pycapio
        pycapio.cpp
)
add_dependencies(_pycapio capio)

target_link_libraries(
        _pycapio
        PRIVATE
        capio_headers
)

target_compile_options(
        _pycapio
        PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

install(
        TARGETS  _pycapio
        LIBRARY DESTINATION pycapio
)

##############################
# Libcapio.cpp example target
#############################

add_executable(libcapio_example libcapio_example.cpp)

add_dependencies(libcapio_example capio)

# 3. Link libraries
# Removed pybind11 references. Added capio_headers.
target_link_libraries(
        libcapio_example
        PRIVATE
        capio_headers
)

# 4. Target compile options (unchanged)
target_compile_options(
        libcapio_example
        PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# 5. Install (Updated)
# For executables, use RUNTIME DESTINATION instead of LIBRARY
install(
        TARGETS libcapio_example
        RUNTIME DESTINATION bin
)
