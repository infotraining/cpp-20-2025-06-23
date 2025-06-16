cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
set(CMAKE_CXX_MODULE_STD 1)

project(primes LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_library(math-with-import-std_lib)

target_sources(math-with-import-std_lib
  PUBLIC
    FILE_SET CXX_MODULES FILES
    math.cxx
    primes.cxx
    fibonacci_seq.cxx
)

add_executable(math-with-import-std math_main.cpp)
target_link_libraries(math-with-import-std PRIVATE math-with-import-std_lib)