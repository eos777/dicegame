cmake_minimum_required(VERSION 3.12)
project(eos777_dice)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(eos777_dice
        include/sevensdice.hpp
        src/sevensdice.cpp
        include/types.hpp
        include/utils.hpp)
