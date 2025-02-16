#ifndef MAGNUM_MOONLANDER_DRAWABLEMESH_H
#define MAGNUM_MOONLANDER_DRAWABLEMESH_H

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Scene.h>

#include "Game.h"

namespace Magnum::Game {
    class DrawableMesh : SceneGraph::Drawable2D {
    public:
        DrawableMesh(
                Object2D &object,
                GL::Mesh& mesh,
                Shaders::Flat2D& shader,
                const Color4 &color,
                SceneGraph::DrawableGroup2D &group
                ) : SceneGraph::Drawable2D{object, &group},
                _mesh(mesh), _shader(shader), _color(color) {}

        void draw(const Matrix3 &transformationMatrix, SceneGraph::Camera2D &camera) override {
            _shader
                    .setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
                    .setColor(_color);

            _shader.draw(_mesh);
        }

    private:
        GL::Mesh& _mesh;
        Shaders::Flat2D& _shader;
        Color4 _color;
    };
}

#endif //MAGNUM_MOONLANDER_DRAWABLEMESH_H
