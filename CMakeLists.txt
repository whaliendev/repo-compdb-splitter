cmake_minimum_required(VERSION 3.21)

project(db-splitter
    VERSION 1.0.0
    LANGUAGES CXX)

option(SP_OUT_TO_BIN "output all target files to build/bin" ON)
option(SP_BUILD_TESTS "build tests" OFF)
option(SP_BUILD_SHARED_LIBS "build shared libraries" OFF)
option(SP_USE_CCACHE "use ccache to accelarate compilation if ccache found in path" ON)
option(SP_USE_MMAP "use mmap to read files" ON)

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    set(SP_BUILD_TESTS ON)
endif(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)

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

if(SP_USE_MMAP)
    add_compile_definitions(SP_WITH_MMAP)
endif(SP_USE_MMAP)

message(STATUS "Build directory: ${PROJECT_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")
    add_compile_definitions(NDEBUG)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# ## conventions we need to follow
if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "The binary directory of CMake cannot be the same as source directory!"
        "Please consider use 'cmake -B build' to specify a different binary directory."
        "Otherwise mergebot may fail to build, or make Git hard to exclude binary files.")
endif(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)

# # solve compatibility problems
if(WIN32)
    set(SP_PLATFORM_NAME SP_PLATFORM_WINDOWS)

    # get rid of 'min', 'max' macros; get back 'M_PI', 'M_E'
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
    add_definitions(-D_UNICODE -DUNICODE)
else(WIN32)
    set(SP_PLATFORM_NAME SP_PLATFORM_POSIX)
    add_compile_definitions(SP_USE_OS_API)
endif(WIN32)

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # MSVC
    # non standard warning,member function not override any super class members,
    # non-virtual-dtor
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive /w14263 /w14265 /W4")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # clang and gcc
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /GL /GS- /fp:fast /fp:except- /MD /Gw /Zc:forScope /Zc:wchar_t /Zc:inline /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Ob0 /Oi /Ot /Oy- /MDd /Gw /Zc:forScope /Zc:wchar_t /Zc:inline /Zc:__cplusplus")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

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
    src/main.cpp
    src/commands.cpp
    include/splitter/consts.h
    include/splitter/commands.h
    include/splitter/util.hpp
    include/splitter/trie.hpp
    include/splitter/filesystem.h)

target_link_libraries(${PROJECT_NAME}
    rapidjson
    onetbb::onetbb
    taywee::args
    rapidxml::rapidxml)

if(SP_BUILD_TESTS)
    message(STATUS "Tests enabled for db-splitter")
    include(cmake/googletest.cmake)

    enable_testing()
    add_subdirectory("third_party/googletest")
    include_directories(${gtest_SOURCE_DIR}/include)
    add_subdirectory(tests)
endif(SP_BUILD_TESTS)