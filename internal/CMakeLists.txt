cmake_minimum_required(VERSION 3.0.0)
project(dugon-sfu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for clang-tidy
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(LOG4 REQUIRED IMPORTED_TARGET liblog4cxx)
pkg_check_modules(FMT REQUIRED IMPORTED_TARGET fmt)

# variable
set(WEBRTC_SRC_PATH "" CACHE STRING "webrtc path")

if(NOT WEBRTC_SRC_PATH)
    message(FATAL_ERROR "WEBRTC_SRC_PATH not provided")
endif()

find_library(WEBRTC libwebrtc.a ${WEBRTC_SRC_PATH}/out/obj)

file(GLOB_RECURSE SRC_FILES src/*.h, src/*.cc)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

set(COMMON_LIBRARIES
    ${WEBRTC}
    PkgConfig::LOG4
    PkgConfig::FMT
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("macos")

    target_compile_definitions(${PROJECT_NAME} PUBLIC WEBRTC_POSIX WEBRTC_MAC)
    find_library(APPLICATION_SERVICES ApplicationServices)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    find_library(CORE_FOUNDATION Foundation)

    target_link_libraries(${PROJECT_NAME}
        ${COMMON_LIBRARIES}

        # for mac
        ${APPLICATION_SERVICES}
        ${AUDIO_TOOLBOX}
        ${CORE_AUDIO}
        ${CORE_FOUNDATION})

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("linux")

    target_compile_definitions(${PROJECT_NAME} PRIVATE WEBRTC_POSIX)
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        ${COMMON_LIBRARIES}
        Threads::Threads)
else()
    message("unknown system " ${CMAKE_SYSTEM_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    src
    ${WEBRTC_SRC_PATH}
    ${WEBRTC_SRC_PATH}/third_party/abseil-cpp
)