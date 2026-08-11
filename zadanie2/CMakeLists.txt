cmake_minimum_required(VERSION 3.10)

project(Zadanie2)

set (CMAKE_CXX_STANDRAD)

find_package(MAVSDK REQUIRED)

add_executable(zadanie2 main.cpp)

target_link_libraries(zadanie2 MAVSDK::mavsdk)
