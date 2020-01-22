cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(gophersrv)

string(APPEND CMAKE_C_FLAGS "-std=c99 ")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -o1 -g -Wall -Wextra -fno-omit-frame-pointer ")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -o3 -DNDEBUG ")

if(UNIX)
    string(APPEND CMAKE_C_FLAGS "-D_GNU_SOURCE -pthread ")

    string(APPEND CMAKE_C_FLAGS_DEBUG "-D_GNU_SOURCE -pthread ")
    string(APPEND CMAKE_C_FLAGS_DEBUG "-fsanitize=address,undefined ")
    #string(APPEND CMAKE_C_FLAGS_DEBUG "-fsanitize=thread ")
    #string(APPEND CMAKE_C_FLAGS_DEBUG "-fsanitize=memory ")
    #string(APPEND CMAKE_C_FLAGS_DEBUG "-fsanitize=safe-stack ")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

add_subdirectory(libs)

if(WIN32)
    add_subdirectory(windows)
else()
    add_subdirectory(linux)
endif()
