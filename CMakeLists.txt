cmake_minimum_required(VERSION 3.15)
project(ErvinCC)

set(CMAKE_CXX_STANDARD 20)

add_executable(ErvinCC main.cpp)
add_executable(parse_rules parse_rules.cpp utils.cpp grammar.cpp)