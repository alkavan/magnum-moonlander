#ifndef MAGNUM_MOONLANDER_LANDER_H
#define MAGNUM_MOONLANDER_LANDER_H


#include "Sprite.h"
#include "SpriteAnimation.h"

namespace Magnum::Game {
    class Lander {
    private:
        Object2D &_object;
        Sprite &_landerSprite;
        Sprite &_engineEffectSprite;
        SpriteAnimation &_engineEffectAnimation;


        Vector2 _thrusterForce = {0.0f, 0.0f};
        Vector2 _thrusterImpulse = {0.0f, 0.0f};

        void thrusterForceToCenter(Vector2 force, const b2BodyId bodyId) const
        {
            b2Body_ApplyForceToCenter(bodyId, b2Vec2{force.x(), force.y()}, true);
        }

        void thrusterImpulseToCenter(Vector2 force, const b2BodyId bodyId) const
        {
            b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{force.x() / 10.0f, force.y() / 10.0f}, true);
        }

    public:
        Lander(
            Object2D &object,
            Sprite &landerSprite,
            Sprite &engineEffectSprite,
            SpriteAnimation &engineEffectAnimation
            ):
        _object(object),
        _landerSprite(landerSprite),
        _engineEffectSprite(engineEffectSprite),
        _engineEffectAnimation(engineEffectAnimation) {}

        [[nodiscard]] Object2D &getObject() const {
            return _object;
        }

        void update(const Float dt, const b2BodyId bodyId) const
        {
            thrusterForceToCenter(_thrusterForce/dt, bodyId);
            // thrusterImpulseToCenter(_thrusterImpulse/dt);

            auto [x, y] = b2Body_GetPosition(bodyId);
            const Float angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));

            (static_cast<Object2D*>(b2Body_GetUserData(bodyId)))
            ->setTranslation({x, y}).setRotation(Complex::rotation(Rad{angle}));

        }

        void addForceX(const Float force) {
            _thrusterForce += Vector2::xAxis(force);
        }

        void addForceY(const Float force) {
            _thrusterForce += Vector2::yAxis(force);
        }

        void resetForceX() {
            _thrusterForce.x() = 0.0f;
        }

        void resetForceY() {
            _thrusterForce.y() = 0.0f;
        }
    };
}

#endif //MAGNUM_MOONLANDER_LANDER_H
