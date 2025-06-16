cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_FLAGS "-stdlib=libc++") # uncomment for Clang

project(cpp_modules_cmake)

add_subdirectory(modules-1)
add_subdirectory(modules-2)
add_subdirectory(modules-3)
# add_subdirectory(drawing-app)
add_subdirectory(import-std/modules-4)
add_subdirectory(import-std/drawing-app)
add_subdirectory(import-std/import-std-lib)