cmake_minimum_required(VERSION 3.10)

project(ZADANIE3)

set(CMAKE_CXX_STANDRAD 20)

find_package(MAVSDK REQUIRED)

add_executable(zadanie3 main.cpp
        HybridInspector.cpp
        HybridInspector.h)

target_link_libraries(zadanie3 MAVSDK::mavsdk)
