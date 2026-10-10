// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_CIRCLE_H
#define MOONLANDER_CIRCLE_H

#include "MoonLander.h"
#include "DrawableMesh.h"

namespace MoonLander {
    /**
     * Visual + physics wrapper for a circle (2D sphere) body.
     * Mirrors Box but is intended for use with circle primitives / circle collision.
     */
    class Circle {
    private:
        Object2D &_object;
        b2BodyId _bodyId;
        DrawableMesh &_drawable;
    public:
        Circle(Object2D &object, const b2BodyId bodyId, DrawableMesh &drawable);
        void update(const Float dt) const;
        b2BodyId bodyId() const { return _bodyId; }
    };
}

#endif // MOONLANDER_CIRCLE_H
