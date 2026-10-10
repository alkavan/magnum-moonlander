# SPDX-FileCopyrightText: 2022-2026 Igal Alkon
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.22)
project(Demos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Magnum CONFIG REQUIRED
        GlfwApplication
        GL
        Shaders
)

add_executable(glfw_app
        GlfwApp.cpp
)

target_link_libraries(glfw_app PRIVATE
        Corrade::Main
        Magnum::GlfwApplication
        Magnum::GL
        Magnum::Magnum
        Magnum::Shaders
)
