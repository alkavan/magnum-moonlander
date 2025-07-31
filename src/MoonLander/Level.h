#ifndef MAGNUM_MOONLANDER_LEVEL_H
#define MAGNUM_MOONLANDER_LEVEL_H

#include <box2d/box2d.h>
#include <Magnum/Math/DualComplex.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Corrade/Containers/GrowableArray.h>

#include "Game.h"
#include "DrawableMesh.h"
#include "Sprite.h"
#include "Lander.h"
#include "Box.h"

namespace Magnum::Game {
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

    inline b2BodyId newWorldObjectBody(
        const b2WorldId worldId,
        Object2D *object,
        const DualComplex &transformation,
        const Vector2 &size,
        const b2BodyType type,
        const Float density
        )
    {
        b2BodyDef bodyDefinition = b2DefaultBodyDef();
        bodyDefinition.position = b2Vec2{transformation.translation().x(), transformation.translation().y()};
        const auto angle = transformation.rotation().angle();
        bodyDefinition.rotation = b2Rot{Math::cos(angle), Math::sin(angle)};
        bodyDefinition.type = type;

        const b2BodyId bodyId = b2CreateBody(worldId, &bodyDefinition);
        b2Body_SetUserData(bodyId, object);

        // bodies.emplace(bodyId, bodyDefinition);

        const b2Polygon shape = b2MakeBox(size.x(), size.y());
        const b2ShapeDef shapeDef = b2DefaultShapeDef();
        // Set friction after shape creation

        const b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &shape);
        b2Shape_SetFriction(shapeId, BodyDefault::friction);
        b2Shape_SetDensity(shapeId, density, true);

        return bodyId;
    }


    class Level {
    private:
        Scene2D& _scene;

        Shaders::FlatGL2D _shader{};
        GL::Mesh _mesh{NoCreate};

        SceneGraph::DrawableGroup2D _boxGroup;
        SceneGraph::DrawableGroup2D _groundGroup;

        b2WorldId _worldId;
        Array<Box*> _boxes{0};
        Optional<Box*> _ground{};
    public:
        Level(Scene2D &scene, const b2WorldId worldId): _scene(scene), _worldId(worldId) {
            _mesh = MeshTools::compile(Primitives::squareSolid());
        }

        Box *newBox(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                    Vector2 size, Color4 color = ObjectDefault::color,
                    Float density = BodyDefault::density);

        Box *newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                          Vector2 size, Color4 color);

        void initialize() {
            _ground = newBoxStatic(
                    _groundGroup,
                    DualComplex::translation(Vector2::yAxis(-10.0f)),
                    {20.0f, 1.0f},
                    0xa5c9ea_rgbf);
        }

        void update(const Float dt) {
            if(_ground) {
                _ground.operator*()->update(dt);
            }

            for (const auto &box : _boxes) {
                box->update(dt);
            }
        }

        void draw(SceneGraph::Camera2D &camera) {
            camera.draw(_groundGroup);
            camera.draw(_boxGroup);
        }

        void addBox(const DualComplex &transformation) {
            const auto box = newBox(
                _boxGroup,
                transformation,
                {0.5f, 0.5f},
                0xffff66_rgbf,
                1.0f);

            arrayAppend(_boxes, box);
        };
    };

    inline Box *Level::newBox(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                              const Vector2 size, const Color4 color, const Float density) {
        const auto object = new Object2D{&_scene};
        object->setScaling(size);
        const auto bodyId = newWorldObjectBody(_worldId, object, transformation, size, b2_dynamicBody, density);
        const auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

        return new Box{*object, bodyId, *drawable};
    }

    inline Box *Level::newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                                    const Vector2 size, const Color4 color) {
        const auto object = new Object2D{&_scene};
        object->setScaling(size);
        const auto bodyId = newWorldObjectBody(_worldId, object, transformation, size, b2_staticBody, 1.0f);
        const auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

        return new Box{*object, bodyId, *drawable};
    }
}

#endif //MAGNUM_MOONLANDER_LEVEL_H