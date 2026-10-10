// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "World.h"

#include <Magnum/Math/Functions.h>
#include <Corrade/Utility/Debug.h>

namespace MoonLander {

World::World(const b2Vec2& gravity) {
    auto worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    _worldId = b2CreateWorld(&worldDef);
}

World::~World() {
    if (b2World_IsValid(_worldId)) {
        b2DestroyWorld(_worldId);
    }
}

void World::step(Float dt, int subSteps) {
    b2World_Step(_worldId, dt, subSteps);
}

b2BodyId World::createBox(
    Object2D* object,
    const DualComplex& transformation,
    const Vector2& size,
    b2BodyType type,
    Float density,
    Float friction
) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{
        .x = transformation.translation().x(),
        .y = transformation.translation().y()
    };
    const auto angle = transformation.rotation().angle();
    bodyDef.rotation = b2Rot{.c = Math::cos(angle), .s = Math::sin(angle)};
    bodyDef.type = type;

    const b2BodyId bodyId = b2CreateBody(_worldId, &bodyDef);
    if (object) {
        b2Body_SetUserData(bodyId, object);
    }

    const b2Polygon polygon = b2MakeBox(size.x(), size.y());
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;

    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    return bodyId;
}

b2BodyId World::createCircle(
    Object2D* object,
    const DualComplex& transformation,
    Float radius,
    b2BodyType type,
    Float density,
    Float friction
) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{
        .x = transformation.translation().x(),
        .y = transformation.translation().y()
    };
    const auto angle = transformation.rotation().angle();
    bodyDef.rotation = b2Rot{.c = Math::cos(angle), .s = Math::sin(angle)};
    bodyDef.type = type;

    const b2BodyId bodyId = b2CreateBody(_worldId, &bodyDef);
    if (object) {
        b2Body_SetUserData(bodyId, object);
    }

    b2Circle circle = b2Circle{.center = {0.0f, 0.0f}, .radius = radius};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;

    b2CreateCircleShape(bodyId, &shapeDef, &circle);
    return bodyId;
}

b2BodyId World::createPolygon(
    Object2D* object,
    const DualComplex& transformation,
    Containers::ArrayView<const Vector2> points,
    b2BodyType type,
    Float density,
    Float friction
) {
    if (points.size() < 3 || points.size() > B2_MAX_POLYGON_VERTICES) {
        Warning{} << "createPolygon: need 3.." << B2_MAX_POLYGON_VERTICES
                  << "points, got" << points.size();
        return b2BodyId{};
    }

    b2Vec2 raw[B2_MAX_POLYGON_VERTICES];
    for (std::size_t i = 0; i < points.size(); ++i) {
        raw[i] = b2Vec2{.x = points[i].x(), .y = points[i].y()};
    }

    const b2Hull hull = b2ComputeHull(raw, static_cast<int>(points.size()));
    if (hull.count < 3) {
        Warning{} << "createPolygon: b2ComputeHull failed (degenerate or collinear)";
        return b2BodyId{};
    }

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{
        .x = transformation.translation().x(),
        .y = transformation.translation().y()
    };
    const auto angle = transformation.rotation().angle();
    bodyDef.rotation = b2Rot{.c = Math::cos(angle), .s = Math::sin(angle)};
    bodyDef.type = type;

    const b2BodyId bodyId = b2CreateBody(_worldId, &bodyDef);
    if (object) {
        b2Body_SetUserData(bodyId, object);
    }

    const b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;

    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    return bodyId;
}

b2BodyId World::createParticle(
    Object2D* object,
    const Vector2& position,
    Float radius,
    Float density
) {
    DualComplex transform = DualComplex::translation(position);
    return createCircle(object, transform, radius, b2_dynamicBody, density, 0.25f);
}

} // namespace MoonLander
