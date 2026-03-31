cmake_minimum_required(VERSION 3.10)
project(modern_cpp_printf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable - the OJ expects this to be named 'code'
add_executable(code main.cpp)
