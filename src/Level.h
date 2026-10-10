// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_LEVEL_H
#define MOONLANDER_LEVEL_H

#include <box2d/box2d.h>
#include <Magnum/Math/DualComplex.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>

#include "MoonLander.h"
#include "DrawableMesh.h"
#include "Sprite.h"
#include "Lander.h"
#include "Box.h"

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

    class BodyDefault {
    public:
        static constexpr Float friction = Float{0.8};
        static constexpr Float density = Float{1.0};
    };

    class ObjectDefault {
    public:
        static constexpr Color4 color = 0xcccccc_rgbf;
    };

    b2BodyId newWorldObjectBody(
        const b2WorldId worldId,
        Object2D *object,
        const DualComplex &transformation,
        const Vector2 &size,
        const b2BodyType type,
        const Float density
    );

    class Level {
    private:
        Scene2D &_scene;

        Shaders::FlatGL2D _shader{};
        GL::Mesh _mesh{NoCreate};

        SceneGraph::DrawableGroup2D _boxGroup;
        SceneGraph::DrawableGroup2D _groundGroup;

        b2WorldId _worldId;
        Array<Box *> _boxes{};
        Optional<Box *> _ground{};

    public:
        Level(Scene2D &scene, const b2WorldId worldId);

        Box *newBox(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                    Vector2 size, Color4 color = ObjectDefault::color,
                    Float density = BodyDefault::density);

        Box *newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                          Vector2 size, Color4 color);

        void initialize();
        void update(const Float dt);
        void draw(SceneGraph::Camera2D &camera);
        void addBox(const DualComplex &transformation);
    };
}

#endif //MOONLANDER_LEVEL_H
