cmake_minimum_required(VERSION 3.10)

project (ZADANIE7)

set (CMAKE_CXX_STANDARDD 23)

find_package(MAVSDK REQUIRED)

add_executable(zadanie7 main.cpp)

target_link_libraries (zadanie7 MAVSDK::mavsdk)

