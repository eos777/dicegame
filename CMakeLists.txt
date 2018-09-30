cmake_minimum_required(VERSION 3.12)
project(dicegame)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(dicegame
        src/sevensdice.cpp)
