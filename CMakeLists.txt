cmake_minimum_required(VERSION 3.12)
project(sevensdice)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(sevensdice
        include/sevensdice.hpp
        include/types.hpp
        include/utils.hpp
        src/sevensdice.cpp)
