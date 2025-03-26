cmake_minimum_required(VERSION 3.22.0)
project(ToT)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

set(SOURCES src/main.cpp src/disk_manager.cpp src/schema_manager.cpp)

add_executable(code_craft ${SOURCES})

set_target_properties(code_craft PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "../"
)