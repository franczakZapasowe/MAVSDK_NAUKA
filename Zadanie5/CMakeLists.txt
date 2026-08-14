cmake_minimum_required(VERSION 3.10)

project(Zadanie5)

set(CMAKE_CXX_STANDARD 20)

find_package(MAVSDK REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(zadanie5 main.cpp
        Dron.cpp
        Dron.h)

target_link_libraries(zadanie5 MAVSDK::mavsdk SQLite::SQLite3)
