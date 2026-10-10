// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Circle.h"

namespace MoonLander {

Circle::Circle(Object2D &object, const b2BodyId bodyId, DrawableMesh &drawable) :
    _object(object), _bodyId(bodyId), _drawable(drawable) {}

void Circle::update(const Float /*dt*/) const {
    if (!b2Body_IsValid(_bodyId)) return;

    const auto [x, y] = b2Body_GetPosition(_bodyId);
    const auto angle = b2Rot_GetAngle(b2Body_GetRotation(_bodyId));

    _object.setTranslation({x, y})
           .setRotation(Complex::rotation(Rad(angle)));
}

} // namespace MoonLander
