cmake_minimum_required(VERSION 3.0.0)
project(dugon-sfu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LOG4 REQUIRED IMPORTED_TARGET liblog4cxx)

#variable
set(WEBRTC_SRC_PATH "" CACHE STRING "webrtc path")

if(NOT WEBRTC_SRC_PATH)
    message(FATAL_ERROR "WEBRTC_SRC_PATH not provided")
endif()

find_library(WEBRTC libwebrtc.a ${WEBRTC_SRC_PATH}/out/obj)

find_package(Boost 1.72.0 COMPONENTS thread system REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.h, src/*.cc)

include_directories(
       src
       deps/json/single_include/nlohmann
       deps 
       ${Boost_INCLUDE_DIRS}
       ${WEBRTC_SRC_PATH}
       ${WEBRTC_SRC_PATH}/third_party/abseil-cpp
)

add_executable(${PROJECT_NAME} 
${SRC_FILES}
main.cc)

set(COMMON_LIBRARIES 
${WEBRTC}
PkgConfig::LOG4
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("macos")

    set(CMAKE_CXX_FLAGS "-DWEBRTC_POSIX -DWEBRTC_MAC ${CMAKE_CXX_FLAGS}")
    find_library(APPLICATION_SERVICES ApplicationServices)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    find_library(CORE_FOUNDATION Foundation)

    target_link_libraries(${PROJECT_NAME}
            ${COMMON_LIBRARIES}
            #for mac
            ${APPLICATION_SERVICES}
            ${AUDIO_TOOLBOX}
            ${CORE_AUDIO}
            ${CORE_FOUNDATION})

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("linux")

    set(CMAKE_CXX_FLAGS "-DWEBRTC_POSIX ${CMAKE_CXX_FLAGS} ")
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME}
            ${COMMON_LIBRARIES}
            Threads::Threads)
else()
    message("unknown system " ${CMAKE_SYSTEM_NAME})
endif()

