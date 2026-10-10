// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Level.h"

#include <Magnum/Math/Functions.h>

namespace MoonLander {

b2BodyId newWorldObjectBody(
    const b2WorldId worldId,
    Object2D *object,
    const DualComplex &transformation,
    const Vector2 &size,
    const b2BodyType type,
    const Float density
) {
    b2BodyDef bodyDefinition = b2DefaultBodyDef();
    bodyDefinition.position = b2Vec2{
        .x = transformation.translation().x(),
        .y = transformation.translation().y()
    };
    const auto angle = transformation.rotation().angle();
    bodyDefinition.rotation = b2Rot{.c = Math::cos(angle), .s = Math::sin(angle)};
    bodyDefinition.type = type;

    const b2BodyId bodyId = b2CreateBody(worldId, &bodyDefinition);
    b2Body_SetUserData(bodyId, object);

    const b2Polygon shape = b2MakeBox(size.x(), size.y());
    const b2ShapeDef shapeDef = b2DefaultShapeDef();

    const b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &shape);
    b2Shape_SetFriction(shapeId, BodyDefault::friction);
    b2Shape_SetDensity(shapeId, density, true);

    return bodyId;
}

Level::Level(Scene2D &scene, const b2WorldId worldId) : _scene(scene), _worldId(worldId) {
    _mesh = MeshTools::compile(Primitives::squareSolid());
}

Box *Level::newBox(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                          const Vector2 size, const Color4 color, const Float density) {
    const auto object = new Object2D{&_scene};
    object->setScaling(size);
    const auto bodyId = newWorldObjectBody(_worldId, object, transformation, size, b2_dynamicBody, density);
    const auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

    return new Box{*object, bodyId, *drawable};
}

Box *Level::newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                                const Vector2 size, const Color4 color) {
    const auto object = new Object2D{&_scene};
    object->setScaling(size);
    const auto bodyId = newWorldObjectBody(_worldId, object, transformation, size, b2_staticBody, 1.0f);
    const auto drawable = new DrawableMesh{*object, _mesh, _shader, color, drawable_group};

    return new Box{*object, bodyId, *drawable};
}

void Level::initialize() {
    _ground = newBoxStatic(
        _groundGroup,
        DualComplex::translation(Vector2::yAxis(-10.0f)),
        {20.0f, 1.0f},
        0xa5c9ea_rgbf);
}

void Level::update(const Float dt) {
    if (_ground) {
        _ground.operator*()->update(dt);
    }

    for (const auto &box: _boxes) {
        box->update(dt);
    }
}

void Level::draw(SceneGraph::Camera2D &camera) {
    camera.draw(_groundGroup);
    camera.draw(_boxGroup);
}

void Level::addBox(const DualComplex &transformation) {
    const auto box = newBox(
        _boxGroup,
        transformation,
        {0.5f, 0.5f},
        0xffff66_rgbf,
        1.0f);

    arrayAppend(_boxes, box);
}

} // namespace MoonLander
