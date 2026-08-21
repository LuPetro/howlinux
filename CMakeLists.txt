cmake_minimum_required(VERSION 3.16)
project(howlinux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# yaml-cpp wird zum Einlesen der meta.yaml Dateien gebraucht
find_package(yaml-cpp REQUIRED)

add_executable(howlinux
    src/main.cpp
    src/knowledge.cpp
    src/search.cpp
    src/render.cpp
)

target_include_directories(howlinux PRIVATE include)
target_link_libraries(howlinux PRIVATE yaml-cpp)
