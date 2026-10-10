// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "DrawableMesh.h"

namespace MoonLander {

DrawableMesh::DrawableMesh(
    Object2D &object,
    GL::Mesh &mesh,
    Shaders::FlatGL2D &shader,
    const Color4 &color,
    SceneGraph::DrawableGroup2D &group
) : SceneGraph::Drawable2D{object, &group},
    _mesh(mesh), _shader(shader), _color(color) {
}

void DrawableMesh::draw(const Matrix3 &transformationMatrix, SceneGraph::Camera2D &camera) {
    _shader
            .setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
            .setColor(_color);

    _shader.draw(_mesh);
}

} // namespace MoonLander
