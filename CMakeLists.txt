cmake_minimum_required(VERSION 3.15)

project(norm)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(
            norm
            norm.cpp
            argparser.hpp
        )

