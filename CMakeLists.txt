cmake_minimum_required(VERSION 3.11)
project(lpg_engine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED LPG_ENGINE_MASTER_PROJECT)
    set(LPG_ENGINE_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(LPG_ENGINE_MASTER_PROJECT ON)
    endif ()
endif ()

file(GLOB_RECURSE LPG_ENGINE_SOURCES "src/*.cpp") 
add_library(lpg_engine ${LPG_ENGINE_SOURCES})
target_include_directories(lpg_engine PUBLIC "src")

find_package(Bullet REQUIRED)
target_include_directories(lpg_engine PUBLIC ${BULLET_INCLUDE_DIRS} PUBLIC "reflect")
target_link_libraries(lpg_engine PUBLIC ${BULLET_LIBRARIES})

find_package(entt REQUIRED)
target_link_libraries(lpg_engine PUBLIC EnTT::EnTT)

find_package(assimp REQUIRED)
target_link_libraries(lpg_engine PRIVATE assimp::assimp)

if(NOT TARGET axxegro)
    option(AXXEGRO_MASTER_PROJECT OFF)
    add_subdirectory(axxegro)
endif ()

get_target_property(debug axxegro INTERFACE_INCLUDE_DIRECTORIES)

if(NOT LPG_ENGINE_MASTER_PROJECT)
    add_subdirectory("demo")
endif ()

target_link_libraries(lpg_engine PUBLIC axxegro)
