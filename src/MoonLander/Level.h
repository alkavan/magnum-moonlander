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
#include "Entity/Lander.h"
#include "Entity/Box.h"

namespace Magnum::Game {
    using namespace Math::Literals;
    using Corrade::Containers::Array;
    using Corrade::Containers::arrayAppend;
    using Corrade::Containers::Optional;

    class BodyDensity {
    // TODO: https://amesweb.info/Materials/Density-Materials.aspx
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
        static inline const Color4 color = 0xcccccc_rgbf;
    };

    inline b2BodyDef createBodyDefinition(const DualComplex &transformation, const b2BodyType type) {
        b2BodyDef bodyDefinition;

        bodyDefinition.position.Set(transformation.translation().x(), transformation.translation().y());
        bodyDefinition.angle = Float(transformation.rotation().angle());
        bodyDefinition.type = type;

        return bodyDefinition;
    }

    inline b2FixtureDef bodyFixtureDefinition(const b2PolygonShape &shape, const Float density) {
        b2FixtureDef fixture;

        fixture.friction = BodyDefault::friction;
        fixture.density = density;
        fixture.shape = &shape;

        return fixture;
    }

    inline b2PolygonShape bodyShape(const Vector2 size) {
        b2PolygonShape shape;
        shape.SetAsBox(size.x(), size.y());
        return shape;
    }

    inline b2Body* newWorldObjectBody(b2World& world,
                                      Object2D &object,
                                      const DualComplex &transformation,
                                      const Vector2 &halfSize,
                                      const b2BodyType type,
                                      const Float density) {

        b2BodyDef bodyDefinition = createBodyDefinition(transformation, type);
        b2Body *body = world.CreateBody(&bodyDefinition);

        b2PolygonShape shape = bodyShape({halfSize});
        b2FixtureDef fixture = bodyFixtureDefinition(shape, density);

        body->CreateFixture(&fixture);
        body->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(&object);
        // for new box2d version
//        body->SetUserData(&object);

        object.setScaling(halfSize);

        return body;
    }

    class Level {
    private:
        Scene2D& _scene;
        b2World& _world;

        Shaders::FlatGL2D _shader{};
        GL::Mesh _mesh{NoCreate};

        SceneGraph::DrawableGroup2D _boxGroup;
        SceneGraph::DrawableGroup2D _groundGroup;

        Array<Box*> _boxes{0};
        Optional<Box*> _ground{};
    public:
        Level(Scene2D &scene, b2World &world): _scene(scene), _world(world) {
            _mesh = MeshTools::compile(Primitives::squareSolid());
        }

        Box *newBox(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                    Vector2 size, Color4 color = ObjectDefault::color,
                    Float density = BodyDefault::density);

        Box *newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                          Vector2 size, Color4 color);

        Lander *newLander(
                Shaders::FlatGL2D &shader,
                SceneGraph::DrawableGroup2D &drawableGroup,
                          GL::Texture2D &landerTexture,
                          GL::Texture2D &engineEffectTexture,
                          DualComplex transformation,
                          Vector2 scale);

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

            for (auto &box : _boxes) {
                box->update(dt);
            }
        }

        void draw(SceneGraph::Camera2D &camera) {
            camera.draw(_groundGroup);
            camera.draw(_boxGroup);
        }

        void addBox(DualComplex &transformation) {
            auto box = newBox(_boxGroup, transformation, {0.5f, 0.5f}, 0xffff66_rgbf, 1.0f);
            arrayAppend(_boxes, box);
        };
    };

    Box *Level::newBox(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                       Vector2 size, Color4 color, Float density) {
        auto object = new Object2D{&_scene};
        auto body = newWorldObjectBody(_world, *object, transformation, size, b2_dynamicBody, density);
        auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

        return new Box{*object, *body, *drawable};

    }

    Box *Level::newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, DualComplex transformation,
                             Vector2 size, Color4 color) {
        auto object = new Object2D{&_scene};
        auto body = newWorldObjectBody(_world, *object, transformation, size, b2_staticBody, 1.0f);
        auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

        return new Box{*object, *body, *drawable};
    }
}

#endif //MAGNUM_MOONLANDER_LEVEL_H
