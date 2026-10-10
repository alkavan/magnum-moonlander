// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_LEVEL_H
#define MOONLANDER_LEVEL_H

#include <Magnum/Math/DualComplex.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Array.h>

#include "MoonLander.h"
#include "World.h"
#include "DrawableMesh.h"
#include "Sprite.h"
#include "Lander.h"
#include "Box.h"
#include "Circle.h"

namespace MoonLander {
    using namespace Math::Literals;
    using Containers::Array;
    using Containers::arrayAppend;
    using Containers::Optional;

    class BodyDensity {
    public:
        static constexpr Float aluminum = Float{168.6f};
        static constexpr Float steel = Float{490.7f};
    };

    class ObjectDefault {
    public:
        static constexpr Color4 color = 0xcccccc_rgbf;
    };

    class Level {
    private:
        Scene2D &_scene;
        World &_world;

        Shaders::FlatGL2D _shader{};
        GL::Mesh _boxMesh{NoCreate};
        GL::Mesh _circleMesh{NoCreate};
        GL::Mesh _terrainMesh{NoCreate};
        GL::Mesh _padMesh{NoCreate};

        SceneGraph::DrawableGroup2D _boxGroup;
        SceneGraph::DrawableGroup2D _groundGroup;
        SceneGraph::DrawableGroup2D _circleGroup;
        SceneGraph::DrawableGroup2D _particleGroup;

        Array<Box *> _boxes{};
        Array<Circle *> _circles{};
        Array<Circle *> _particles{};

        Object2D* _terrainObject = nullptr;
        DrawableMesh* _terrainDrawable = nullptr;
        Object2D* _padObject = nullptr;
        DrawableMesh* _padDrawable = nullptr;

        void buildMoonLandscape();

    public:
        Level(Scene2D &scene, World &world);

        Box *newBox(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                    Vector2 size, Color4 color = ObjectDefault::color,
                    Float density = BodyDefault::density);

        Box *newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                          Vector2 size, Color4 color);

        Circle *newCircle(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                          Float radius, Color4 color = ObjectDefault::color,
                          Float density = BodyDefault::density);

        void initialize();
        void update(Float dt);
        void draw(SceneGraph::Camera2D &camera);

        void addBox(const DualComplex &transformation);
        void addCircle(const DualComplex &transformation, Float radius = 0.4f);
        void addParticleCloud(const Vector2 &center, int count = 40, Float spread = 1.5f);
    };
}

#endif //MOONLANDER_LEVEL_H
