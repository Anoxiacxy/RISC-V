# CMake 最低版本号要求

cmake_minimum_required (VERSION 2.8)

# CMake 编译选项

set(CMAKE_CXX_FLAGS "-O2")

# 项目信息

project (RISC-V)

# 指定生成目标

add_executable(code release.cpp)