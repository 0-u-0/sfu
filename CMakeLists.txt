cmake_minimum_required(VERSION 3.0.0)
project(dugon-sfu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.72.0 COMPONENTS thread system REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.h, src/*.cc)

include_directories(
       src
       deps/json/single_include/nlohmann
       deps 
       ${Boost_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} 
${SRC_FILES}
main.cc)

