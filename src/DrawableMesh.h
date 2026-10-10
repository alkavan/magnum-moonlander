// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_DRAWABLEMESH_H
#define MOONLANDER_DRAWABLEMESH_H

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Scene.h>

#include "MoonLander.h"

namespace MoonLander {
    class DrawableMesh final : SceneGraph::Drawable2D {
    public:
        DrawableMesh(
            Object2D &object,
            GL::Mesh &mesh,
            Shaders::FlatGL2D &shader,
            const Color4 &color,
            SceneGraph::DrawableGroup2D &group
        );

        void draw(const Matrix3 &transformationMatrix, SceneGraph::Camera2D &camera) override;

    private:
        GL::Mesh &_mesh;
        Shaders::FlatGL2D &_shader;
        Color4 _color;
    };
}

#endif //MOONLANDER_DRAWABLEMESH_H
