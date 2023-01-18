
file(GLOB_RECURSE LPG_ENGINE_DEMO_SOURCES "src/*.cpp")


add_executable(lpg_engine_demo ${LPG_ENGINE_DEMO_SOURCES})
target_link_libraries(lpg_engine_demo lpg_engine)

add_custom_command(TARGET lpg_engine_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data/ ${CMAKE_CURRENT_BINARY_DIR}/data
)

