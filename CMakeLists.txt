# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

if(${CMAKE_VERSION} VERSION_EQUAL "3.14" OR ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

project(aff3ct CXX)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE OPTIONS
# ---------------------------------------------------------------------------------------------------------------------

option(AFF3CT_COMPILE_EXE        "Compile the executable"                                                   ON )
option(AFF3CT_MPI                "Enable the MPI support"                                                   OFF)
option(AFF3CT_OVERRIDE_VERSION   "Compile without .git directory, provided a version and hash"              OFF)
option(AFF3CT_INCLUDE_SPU_LIB    "Include the StreamPU library inside the AFF3CT library"                   ON )

if (AFF3CT_COMPILE_EXE OR AFF3CT_COMPILE_STATIC_LIB OR AFF3CT_COMPILE_SHARED_LIB)
    set(AFF3CT_COMPILE_OBJ ON)
else()
    set(AFF3CT_COMPILE_OBJ OFF)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" AND NOT AFF3CT_OVERRIDE_VERSION)
    message(FATAL_ERROR "The '.git' folder can't be found, AFF3CT can't be compiled if it is not cloned "
                        "from a Git repository. Please do not download archives from GitHub and make a Git "
                        "clone instead (git clone https://github.com/aff3ct/aff3ct.git).")
endif()

if(MSVC)
    set(AFF3CT_EXT_STRINGS ON)
else()
    option(AFF3CT_EXT_STRINGS "Enable external strings for the help documentation" ON)
endif()

set(AFF3CT_PREC "MULTI" CACHE STRING "Select the precision in bits (can be '8', '16', '32', '64' or 'MULTI')")

if(AFF3CT_MPI AND (AFF3CT_COMPILE_STATIC_LIB OR AFF3CT_COMPILE_SHARED_LIB))
    message(FATAL_ERROR "Building AFF3CT with the MPI support is incompatible with the library mode.")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------- CMAKE CONFIGURATION
# ---------------------------------------------------------------------------------------------------------------------

# set CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_INCLUDEDIR and CMAKE_INSTALL_DATAROOTDIR variables
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_MACOSX_RPATH 1)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify bin and lib paths
set(EXECUTABLE_OUTPUT_PATH bin/)
set(LIBRARY_OUTPUT_PATH lib/)

# Generate the source files list
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)


# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------ GENERATE FILES
# ---------------------------------------------------------------------------------------------------------------------

# ---------------------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------------- SUB-PROJECT
# ---------------------------------------------------------------------------------------------------------------------

if (AFF3CT_INCLUDE_SPU_LIB)
    # StreamPU
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/streampu/CMakeLists.txt")
        option(SPU_COMPILE_STATIC_LIB "" OFF)
        option(SPU_COMPILE_SHARED_LIB "" OFF)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/streampu/ EXCLUDE_FROM_ALL)
        message(STATUS "AFF3CT - Sub-project found: StreamPU")
    else()
        message(FATAL_ERROR "AFF3CT - StreamPU can't be found, try to init the submodule with the following cmd:\n"
                            "$ git submodule update --init --recursive")
    endif()
endif()

# cli
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/cli/src/cli.hpp")
    option(CLI_COMPILE_STATIC_LIB "" OFF)
    option(CLI_COMPILE_SHARED_LIB "" OFF)
    option(CLI_STATUS_MESSAGE_QUIET "" ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/cli/ EXCLUDE_FROM_ALL)
    message(STATUS "AFF3CT - Sub-project found: cli")
else()
    message(FATAL_ERROR "AFF3CT - cli can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init --recursive")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------- OBJECTS/LIBS/EXE
# ---------------------------------------------------------------------------------------------------------------------



# Object
if(AFF3CT_COMPILE_OBJ)
    add_library(aff3ct-obj OBJECT ${source_files})
    set_target_properties(aff3ct-obj PROPERTIES
                                     POSITION_INDEPENDENT_CODE ON) # set -fpic
endif()

# Binary
if(AFF3CT_COMPILE_EXE)
    if(AFF3CT_INCLUDE_SPU_LIB)
        add_executable(aff3ct-bin $<TARGET_OBJECTS:aff3ct-obj> $<TARGET_OBJECTS:cli-obj> $<TARGET_OBJECTS:spu-obj>)
    else()
        add_executable(aff3ct-bin $<TARGET_OBJECTS:aff3ct-obj> $<TARGET_OBJECTS:cli-obj>)
    endif()
    set_target_properties(aff3ct-bin PROPERTIES
                                     OUTPUT_NAME aff3ct-${AFF3CT_VERSION_FULL}
                                     POSITION_INDEPENDENT_CODE ON) # set -fpie
    message(STATUS "AFF3CT - Compile: executable")
endif(AFF3CT_COMPILE_EXE)



# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------ COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_target_compile_definitions privacy def)
    if(AFF3CT_COMPILE_OBJ)
        target_compile_definitions(aff3ct-obj ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endif()
    if(AFF3CT_COMPILE_EXE)
        target_compile_definitions(aff3ct-bin ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endif()
endmacro()



if(AFF3CT_EXT_STRINGS)
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_EXT_STRINGS)
    message(STATUS "AFF3CT - External strings: on")
else()
    message(STATUS "AFF3CT - External strings: off")
endif()

if(AFF3CT_PREC STREQUAL "8")
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_8BIT_PREC)
    message(STATUS "AFF3CT - Precision: 8-bit")
elseif(AFF3CT_PREC STREQUAL "16")
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_16BIT_PREC)
    message(STATUS "AFF3CT - Precision: 16-bit")
elseif(AFF3CT_PREC STREQUAL "32")
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_32BIT_PREC)
    message(STATUS "AFF3CT - Precision: 32-bit")
elseif(AFF3CT_PREC STREQUAL "64")
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_64BIT_PREC)
    message(STATUS "AFF3CT - Precision: 64-bit")
elseif(AFF3CT_PREC STREQUAL "MULTI")
    aff3ct_target_compile_definitions(PUBLIC AFF3CT_MULTI_PREC)
    message(STATUS "AFF3CT - Precision: 8/16/32/64-bit")
else()
    message(FATAL_ERROR "AFF3CT_PREC='${AFF3CT_PREC}' and should be '8', '16', '32', '64' or 'MULTI'.")
endif()

if (AFF3CT_INCLUDE_SPU_LIB)
    if(SPU_COLORS)
        aff3ct_target_compile_definitions(PUBLIC SPU_COLORS)
    endif()
    if(SPU_STACKTRACE)
        aff3ct_target_compile_definitions(PUBLIC SPU_STACKTRACE)
    endif()
    if(SPU_STACKTRACE_SEGFAULT)
        aff3ct_target_compile_definitions(PUBLIC SPU_STACKTRACE_SEGFAULT)
        if (CPPTRACE_UNWIND_WITH_LIBUNWIND)
            aff3ct_target_compile_definitions(PUBLIC SPU_STACKTRACE_SEGFAULT_LIBUNWIND)
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------- HEADER ONLY LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_target_include_directories privacy dir_build dir_install)
    if(AFF3CT_COMPILE_OBJ)
        target_include_directories(aff3ct-obj ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-${AFF3CT_VERSION_FULL}/${dir_install}>)
    endif()
    if(AFF3CT_COMPILE_EXE)
        target_include_directories(aff3ct-bin ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-${AFF3CT_VERSION_FULL}/${dir_install}>)
    endif()
endmacro()

macro(aff3ct_target_include_directories2 privacy dir)
    if(AFF3CT_COMPILE_OBJ)
        target_include_directories(aff3ct-obj ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endif()
    if(AFF3CT_COMPILE_EXE)
        target_include_directories(aff3ct-bin ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endif()
endmacro()

# AFF3CT headers
aff3ct_target_include_directories(PRIVATE "src"     "aff3ct")
aff3ct_target_include_directories(PUBLIC  "include" "aff3ct")

# StreamPU headers
if(AFF3CT_INCLUDE_SPU_LIB)
    aff3ct_target_include_directories(PUBLIC  "lib/streampu/include"              "streampu")
    aff3ct_target_include_directories(PRIVATE "lib/streampu/src"                  "streampu")
    aff3ct_target_include_directories(PUBLIC  "lib/streampu/lib/rang/include"     "rang"    )
    aff3ct_target_include_directories(PUBLIC  "lib/streampu/lib/cpptrace/include" "cpptrace")
    aff3ct_target_include_directories(PUBLIC  "lib/streampu/lib/json/include"     "json"    )
endif()

# cli headers
aff3ct_target_include_directories(PUBLIC "lib/cli/src" "cli")

# MIPP
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/MIPP/src/mipp.h")
    aff3ct_target_compile_definitions(PUBLIC MIPP_ENABLE_BACKTRACE)
    aff3ct_target_include_directories(PUBLIC "lib/MIPP/src" "MIPP")
    message(STATUS "AFF3CT - Header found: MIPP")
else()
    message(FATAL_ERROR "AFF3CT - MIPP can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/MIPP/")
endif()

# date
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/date/include/date/date.h")
    aff3ct_target_include_directories(PUBLIC "lib/date/include/date" "date")
    message(STATUS "AFF3CT - Header found: date")
else()
    message(FATAL_ERROR "AFF3CT - date can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/date/")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------- COMPILED LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_target_link_libraries privacy lib)
    if(AFF3CT_COMPILE_EXE)
        target_link_libraries(aff3ct-bin ${privacy} ${lib})
    endif(AFF3CT_COMPILE_EXE)
endmacro()

# cpptrace
if (AFF3CT_INCLUDE_SPU_LIB)
    if(SPU_STACKTRACE OR SPU_STACKTRACE_SEGFAULT)
        aff3ct_target_link_libraries(PUBLIC cpptrace::cpptrace)
    endif()
else()
    if (TARGET cpptrace::cpptrace)
        message(STATUS "AFF3CT - Library found: cpptrace (from CMake prev. target!)")
        aff3ct_target_link_libraries(PUBLIC cpptrace::cpptrace)
    else()
        find_package(cpptrace CONFIG 0.4.1)
        if (cpptrace_FOUND)
            message(STATUS "AFF3CT - Library found: cpptrace")
            aff3ct_target_link_libraries(PUBLIC cpptrace::cpptrace)
        endif()
    endif()
endif()

# StreamPU
if (NOT AFF3CT_INCLUDE_SPU_LIB)
    if (TARGET spu-static-lib)
        message(STATUS "AFF3CT - Library found: StreamPU (from CMake prev. target!)")
        target_link_libraries(aff3ct-obj PUBLIC spu-static-lib)
        aff3ct_target_link_libraries(PUBLIC spu-static-lib)
    else()
        find_package(StreamPU CONFIG 1.0.2 REQUIRED)
        message(STATUS "AFF3CT - Library found: StreamPU")
        if (AFF3CT_COMPILE_SHARED_LIB)
            target_link_libraries(aff3ct-obj PUBLIC spu::spu-shared-lib)
            aff3ct_target_link_libraries(PUBLIC spu::spu-shared-lib)
        else()
            target_link_libraries(aff3ct-obj PUBLIC spu::spu-static-lib)
            aff3ct_target_link_libraries(PUBLIC spu::spu-static-lib)
        endif()
    endif()
endif()

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
aff3ct_target_link_libraries(PUBLIC Threads::Threads)


