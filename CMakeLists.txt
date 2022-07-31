cmake_minimum_required(VERSION 3.9)
project(sfu-addon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WEBRTC_SRC_PATH "" CACHE STRING "webrtc src path")

# set(WEBRTC_PACKAGE_PATH ${CMAKE_SOURCE_DIR}/third_party/webrtc-collect/package CACHE STRING "webrtc package path")
# set(DISABLE_H264_DECODER "ON" CACHE STRING "disable h264 decoder")
message("WEBRTC_SRC_PATH " ${WEBRTC_SRC_PATH})

# message("WEBRTC_PACKAGE_PATH " ${WEBRTC_PACKAGE_PATH})

# internal
add_subdirectory(internal)

# source files
file(GLOB SOURCE_FILES "addon/*.cc" "addon/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB}
  dugon-sfu
)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR
)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
message(STATUS "NODE_ADDON_API_DIR: " ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_JS_INC}
  ${NODE_ADDON_API_DIR}
  "addon")