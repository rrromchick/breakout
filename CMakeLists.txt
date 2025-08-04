cmake_minimum_required(VERSION 3.10)

project(BREAKOUT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(
    glfw 
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    assimp 
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.0
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG VER-2-13-2
)
FetchContent_MakeAvailable(freetype)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

file(GLOB GLAD_SOURCES "${GLAD_DIR}/glad.c")
add_library(glad STATIC ${GLAD_SOURCES})
target_include_directories(glad PUBLIC "${GLAD_DIR}" "{GLAD_DIR}/glad" "${GLAD_DIR}/KHR")

add_executable(main main.cpp game.h game.cpp resource_manager.h resource_manager.cpp
    shader.h shader.cpp stb_image.h texture.h texture.cpp
    sprite_renderer.h sprite_renderer.cpp file_system.h game_object.h game_object.cpp
    game_level.h game_level.cpp)

target_link_libraries(main PRIVATE glfw glad glm ${CMAKE_DL_LIBS} assimp freetype)

target_include_directories(main PUBLIC ${GLAD_DIR} ${glfw_SOURCE_DIR}/include)