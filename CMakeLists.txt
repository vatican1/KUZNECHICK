cmake_minimum_required(VERSION 3.5)

project(Kuznechik LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options("-o2")

add_executable(Kuznechik main.cpp linear.cpp linear.h non_linear.cpp non_linear.h consts.h)
