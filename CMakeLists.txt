cmake_minimum_required(VERSION 3.15)
project(magnum_moonlander)

set(CMAKE_CXX_STANDARD 17)

# Add module path in case this is project root
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
endif()

find_package(Corrade REQUIRED
        Main
)

find_package(Magnum REQUIRED
        GL
        MeshTools
        Primitives
        SceneGraph
        Sdl2Application
        Shaders
        Trade
        Audio
)

find_package(Box2D CONFIG REQUIRED)
find_package(OpenAL)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(MoonLander_RESOURCES res/resources.conf)

add_executable(magnum_moonlander
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

target_link_libraries(magnum_moonlander PRIVATE
        Corrade::Main
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
)

# Link
target_link_libraries(magnum_moonlander PRIVATE
        OpenAL::OpenAL
)

# Link Box2D
if (WIN32)  # Windows
    target_link_libraries(magnum_moonlander PRIVATE box2d::box2d)
else()      # Linux and Mac
    target_link_libraries(magnum_moonlander PRIVATE Box2D::Box2D)
endif (WIN32)

#install(TARGETS magnum_moonlander DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})
