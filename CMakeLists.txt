cmake_minimum_required(VERSION 3.11)
project(lpg_engine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE LPG_ENGINE_SOURCES "src/*.cpp") 

add_library(lpg_engine ${LPG_ENGINE_SOURCES})
