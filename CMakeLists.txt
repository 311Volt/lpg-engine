cmake_minimum_required(VERSION 3.11)
project(lpg_engine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LPG_ENGINE_MASTER_PROJECT "Set this to OFF if using as submodule" ON)

file(GLOB_RECURSE LPG_ENGINE_SOURCES "src/*.cpp") 
add_library(lpg_engine ${LPG_ENGINE_SOURCES})
target_include_directories(lpg_engine PUBLIC "include")


option(AXXEGRO_MASTER_PROJECT OFF)
add_subdirectory(axxegro)

add_subdirectory("demo")

target_link_libraries(lpg_engine PUBLIC axxegro)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


