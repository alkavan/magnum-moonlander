cmake_minimum_required(VERSION 3.15)
project(lander LANGUAGES CXX VERSION "0.1.0")

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

find_package(Box2D CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(MoonLander_RESOURCES res/resources.conf)

add_executable(lander
        ${MoonLander_RESOURCES}
        src/game.cpp
        src/MoonLander/AssetManager.h
        src/MoonLander/DrawableMesh.h
        src/MoonLander/Game.h
        src/MoonLander/Level.h
        src/MoonLander/CameraControl.h
        src/MoonLander/Sprite.h
        src/MoonLander/SpriteAnimation.h
        src/MoonLander/Lander.h
        src/MoonLander/Box.h
)

target_link_libraries(lander PRIVATE
        Corrade::Main
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        MagnumExtras::Ui
        OpenAL::OpenAL
)

# Link Box2D
if (WIN32)  # Windows
    target_link_libraries(lander PRIVATE box2d::box2d)
else()      # Linux and Mac
    target_link_libraries(lander PRIVATE Box2D::Box2D)
endif (WIN32)

#install(TARGETS lander DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})
