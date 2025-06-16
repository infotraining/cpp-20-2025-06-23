cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
set(CMAKE_CXX_MODULE_STD 1)

project(import-std-lib LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_executable(import-std-lib main.cpp)
