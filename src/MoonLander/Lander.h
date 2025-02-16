#ifndef MAGNUM_MOONLANDER_LANDER_H
#define MAGNUM_MOONLANDER_LANDER_H


#include "Sprite.h"
#include "SpriteAnimation.h"

namespace Magnum::Game {
    class Lander {
    private:
        Object2D &_object;
        b2Body &_body;
        Sprite &_landerSprite;
        Sprite &_engineEffectSprite;
        SpriteAnimation &_engineEffectAnimation;


        Vector2 _thrusterForce = {0.0f, 0.0f};
        Vector2 _thrusterImpulse = {0.0f, 0.0f};

        void thrusterForceToCenter(Vector2 force) const
        {
            _body.ApplyForceToCenter(b2Vec2{force.x(), force.y()}, true);
        }

        void thrusterImpulseToCenter(Vector2 force) const
        {
            _body.ApplyLinearImpulseToCenter(b2Vec2{force.x() / 10.0f, force.y() / 10.0f}, true);
        }

    public:
        Lander(
            Object2D &object,
            b2Body &body,
            Sprite &landerSprite,
            Sprite &engineEffectSprite,
            SpriteAnimation &engineEffectAnimation
            ):
        _object(object),
        _body(body),
        _landerSprite(landerSprite),
        _engineEffectSprite(engineEffectSprite),
        _engineEffectAnimation(engineEffectAnimation) {}

        [[nodiscard]] Object2D &getObject() const {
            return _object;
        }

        [[nodiscard]] b2Body &getBody() const {
            return _body;
        }

        [[nodiscard]] Vector2 getVelocity() const {
            auto velocity = _body.GetLinearVelocity();
            return {velocity.x, velocity.y};
        }

        void update(const Float dt) const
        {
            thrusterForceToCenter(_thrusterForce/dt);
//            thrusterImpulseToCenter(_thrusterImpulse/dt);

            (*reinterpret_cast<Object2D*>(_body.GetUserData().pointer))
                    .setTranslation({_body.GetPosition().x, _body.GetPosition().y})
                    .setRotation(Complex::rotation(Rad(_body.GetAngle())));
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
