# Config is only used to enable CLion features!
# Do not use this to build!
cmake_minimum_required(VERSION 3.20)
project(godot_steamworks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES src/*.c**)
file(GLOB_RECURSE HEADERS src/*.h**)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        godot-cpp/include
        godot-cpp/godot-headers
        godot-cpp/gen/include
        sdk/public
)
