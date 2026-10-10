// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Lander.h"

namespace MoonLander {

void Lander::thrusterForceToCenter(Vector2 force, const b2BodyId bodyId) {
    b2Body_ApplyForceToCenter(bodyId, b2Vec2{.x = force.x(), .y = force.y()}, true);
}

void Lander::thrusterImpulseToCenter(Vector2 force, const b2BodyId bodyId) {
    b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{.x = force.x() / 10.0f, .y = force.y() / 10.0f}, true);
}

Lander::Lander(
    Object2D &object,
    Sprite &landerSprite,
    Sprite &engineEffectSprite,
    SpriteAnimation &engineEffectAnimation
) : _object(object),
    _landerSprite(landerSprite),
    _engineEffectSprite(engineEffectSprite),
    _engineEffectAnimation(engineEffectAnimation) {
}

Object2D &Lander::getObject() const {
    return _object;
}

void Lander::update(const Float dt, const b2BodyId bodyId) const {
    thrusterForceToCenter(_thrusterForce / dt, bodyId);

    auto [x, y] = b2Body_GetPosition(bodyId);
    const Float angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));

    (static_cast<Object2D *>(b2Body_GetUserData(bodyId)))
            ->setTranslation({x, y}).setRotation(Complex::rotation(Rad{angle}));
}

void Lander::addForceX(const Float force) {
    _thrusterForce += Vector2::xAxis(force);
}

void Lander::addForceY(const Float force) {
    _thrusterForce += Vector2::yAxis(force);
}

void Lander::resetForceX() {
    _thrusterForce.x() = 0.0f;
}

void Lander::resetForceY() {
    _thrusterForce.y() = 0.0f;
}

} // namespace MoonLander
