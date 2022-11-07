cmake_minimum_required(VERSION 3.15)
project(magnum_moonlander)

set(CMAKE_CXX_STANDARD 17)

# Add module path in case this is project root
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
endif()

find_package(Corrade REQUIRED Main)

find_package(Magnum REQUIRED
        GL
        MeshTools
        Primitives
        SceneGraph
        Sdl2Application
        Shaders
        Trade)

find_package(Box2D REQUIRED)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(MoonLander_RESOURCES res/resources.conf)

add_executable(magnum-moonlander
        ${MoonLander_RESOURCES}
        src/game.cpp
        src/MoonLander/AssetManager.h
        src/MoonLander/DrawableMesh.h
        src/MoonLander/Game.h
        src/MoonLander/Level.h
        src/MoonLander/CameraControl.h
        src/MoonLander/Sprite.h
        src/MoonLander/SpriteAnimation.h
        src/MoonLander/Entity/Lander.h
        src/MoonLander/Entity/Box.h
        )

target_link_libraries(magnum-moonlander PRIVATE
        Corrade::Main
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        Box2D::Box2D)

#install(TARGETS magnum-moonlander DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})
