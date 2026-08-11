cmake_minimum_required(VERSION 3.10)

project(Zadanie1)

set (CMAKE_CXX_STANDRAD 20)

find_package(MAVSDK REQUIRED)

add_executable(zadanie1 main.cpp)

target_link_libraries(zadanie1 MAVSDK::mavsdk)
