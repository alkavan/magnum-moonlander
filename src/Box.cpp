// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Box.h"

namespace MoonLander {

Box::Box(Object2D &object, const b2BodyId bodyId, DrawableMesh &drawable) :
    _object(object), _bodyId(bodyId), _drawable(drawable) {}

void Box::update(const Float dt) const {
    const auto userData = b2Body_GetUserData(_bodyId);
    const auto [x, y] = b2Body_GetPosition(_bodyId);
    const auto angle = b2Rot_GetAngle(b2Body_GetRotation(_bodyId));

    static_cast<Object2D*>(userData)->setTranslation({x, y})
        .setRotation(Complex::rotation(Rad(angle)));
}

} // namespace MoonLander
