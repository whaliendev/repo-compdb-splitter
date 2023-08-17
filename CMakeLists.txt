cmake_minimum_required(VERSION 3.21)

project(db-splitter
    VERSION 1.0.0
    LANGUAGES CXX)

option(SP_OUT_TO_BIN "output all target files to build/bin" ON)
option(SP_BUILD_TESTS "build tests" OFF)
option(SP_BUILD_SHARED_LIBS "build shared libraries" OFF)
option(SP_USE_CCACHE "use ccache to accelarate compilation if ccache found in path" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ")
set(CMAKE_LINK_WHAT_YOU_USE ON)

# C++ standard can also be overridden when this is used as a sub-project
if(NOT CMAKE_CXX_STANDARD)
    # we can use all C++17 features in this project
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

# enable dynamic lib to link static lib
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    # Otherwise we cannot link .so libs with .a libs
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)

# the default build target is set to Release
if(NOT CMAKE_BUILD_TYPE)
    # Default build target
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build directory: ${PROJECT_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ## conventions we need to follow
if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "The binary directory of CMake cannot be the same as source directory!"
        "Please consider use 'cmake -B build' to specify a different binary directory."
        "Otherwise mergebot may fail to build, or make Git hard to exclude binary files.")
endif(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)

# ## solve compatibility problems
# if(WIN32)
# set(SP_PLATFORM_NAME SP_PLATFORM_WINDOWS)

# # get rid of 'min', 'max' macros; get back 'M_PI', 'M_E'
# add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
# add_definitions(-D_UNICODE -DUNICODE)
# else(WIN32)
# set(SP_PLATFORM_NAME SP_PLATFORM_POSIX)
# endif(WIN32)
if(SP_OUT_TO_BIN)
    # Otherwise Windows (which doesn't support RPATH) cannot find the DLLs at runtime
    set(SP_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SP_OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SP_OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SP_OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${SP_OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SP_OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${SP_OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${SP_OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${SP_OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${SP_OUTPUT_DIR})
    set(CMAKE_DEBUG_POSTFIX "") # No more annoying SP_d.dll
endif()

# global include directories
include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

# ## our preferences
# make compiler be a grudge in debug mode
# speed up compilation by cache(https://ccache.dev)
if(SP_USE_CCACHE AND NOT MSVC)
    find_program(CCACHE_PROGRAM ccache)

    if(CCACHE_PROGRAM)
        message(STATUS "Found CCache: ${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif(CCACHE_PROGRAM)
endif(SP_USE_CCACHE AND NOT MSVC)

if(SP_BUILD_SHARED_LIBS)
    # Only export SP_EXPORT symbols from the shared library.
    add_compile_options(-fvisibility=hidden)
    target_compile_definitions(mergebot
        PUBLIC

        # Used by include/export.h.
        SP_SHARED_LIBRARY
    )
endif(SP_BUILD_SHARED_LIBS)

# ## targets go here
find_package(RapidJSON REQUIRED)
find_package(TBB REQUIRED)
find_package(args REQUIRED)
find_package(rapidxml REQUIRED)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/main.cpp)

target_link_libraries(${PROJECT_NAME}
    rapidjson
    onetbb::onetbb
    taywee::args
    rapidxml::rapidxml)