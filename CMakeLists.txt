cmake_minimum_required(VERSION 3.22)
project(MagnumLander LANGUAGES CXX VERSION 0.1.2)

set(CMAKE_CXX_STANDARD 20)

# Add module path in case this is project root
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
endif()

# generated code
configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories(${CMAKE_BINARY_DIR}/generated/)

find_package(Corrade CONFIG REQUIRED Main)

find_package(Magnum CONFIG REQUIRED
        GL
        MeshTools
        Primitives
        SceneGraph
        Sdl2Application
        Shaders
        Trade
        Audio
)

find_package(MagnumExtras REQUIRED Ui)

include(FetchContent)
FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v3.1.1
        GIT_SHALLOW TRUE
)
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_PROFILE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(box2d)
if(NOT TARGET box2d::box2d)
    add_library(box2d::box2d ALIAS box2d)
endif()

find_package(OpenAL CONFIG REQUIRED)

corrade_add_resource(MoonLander_RESOURCES res/resources.conf)

add_executable(magnum_lander
        ${MoonLander_RESOURCES}
        src/MoonLanderGame.cpp
)

target_link_libraries(magnum_lander PRIVATE
        Corrade::Main
        Magnum::Sdl2Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        MagnumExtras::Ui
        OpenAL::OpenAL
        box2d::box2d
)
