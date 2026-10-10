// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_WORLD_H
#define MOONLANDER_WORLD_H

#include <box2d/box2d.h>
#include <Magnum/Math/DualComplex.h>
#include <Corrade/Containers/ArrayView.h>

#include "MoonLander.h"

namespace MoonLander {

    class BodyDefault {
    public:
        static constexpr Float friction = Float{0.8f};
        static constexpr Float density = Float{1.0f};
    };

    /**
     * Thin wrapper around a Box2D v3 world that creates rigid bodies with
     * box, circle (sphere), polygon, and particle shapes.
     *
     * Box2D 3.x (the version this project uses) provides:
     *  - Polygons via b2MakeBox / b2ComputeHull + b2MakePolygon + b2CreatePolygonShape
     *    (max B2_MAX_POLYGON_VERTICES = 8, must be convex)
     *  - Circles via b2CreateCircleShape (the 2D equivalent of a sphere)
     *  - Capsules, segments, and chains
     *
     * Official Box2D does **not** ship a particle system. LiquidFun added
     * SPH-style fluid particles; a community port to Box2D 3.x exists but is
     * not linked here. Particles in this World are therefore implemented as
     * many small dynamic circle bodies. This is appropriate for debris, dust,
     * sparks, and simple granular effects. For true fluid simulation one would
     * integrate LiquidFun or a similar solver.
     */
    class World {
    public:
        explicit World(const b2Vec2& gravity = GravityConstant::Moon);
        ~World();

        World(const World&) = delete;
        World& operator=(const World&) = delete;

        /** Step the simulation. */
        void step(Float dt, int subSteps = 4);

        b2WorldId id() const { return _worldId; }

        /**
         * Create a box body.
         * size is half-extents matching the b2MakeBox convention.
         * The Magnum Object2D is stored as user data for later transform sync.
         */
        b2BodyId createBox(
            Object2D* object,
            const DualComplex& transformation,
            const Vector2& size,
            b2BodyType type = b2_dynamicBody,
            Float density = BodyDefault::density,
            Float friction = BodyDefault::friction
        );

        /**
         * Create a circle / 2D sphere body.
         */
        b2BodyId createCircle(
            Object2D* object,
            const DualComplex& transformation,
            Float radius,
            b2BodyType type = b2_dynamicBody,
            Float density = BodyDefault::density,
            Float friction = BodyDefault::friction
        );

        /**
         * Create a convex polygon body from an arbitrary set of points.
         *
         * Points are in local space relative to the body's transform.
         * Box2D will compute a convex hull (b2ComputeHull). The result must
         * have at least 3 vertices and at most B2_MAX_POLYGON_VERTICES (8).
         * Collinear or degenerate sets return an invalid body id.
         *
         * Typical use: arbitrary convex shapes, or terrain slabs.
         */
        b2BodyId createPolygon(
            Object2D* object,
            const DualComplex& transformation,
            Containers::ArrayView<const Vector2> points,
            b2BodyType type = b2_staticBody,
            Float density = BodyDefault::density,
            Float friction = BodyDefault::friction
        );

        /**
         * Create a particle (small dynamic circle).
         * Suitable for debris, sparks, dust, etc.
         */
        b2BodyId createParticle(
            Object2D* object,
            const Vector2& position,
            Float radius = 0.08f,
            Float density = 0.4f
        );

    private:
        b2WorldId _worldId{};
    };
}

#endif // MOONLANDER_WORLD_H
