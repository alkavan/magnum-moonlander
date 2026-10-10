// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_LANDER_H
#define MOONLANDER_LANDER_H

#include "MoonLander.h"
#include "Sprite.h"
#include "SpriteAnimation.h"

namespace MoonLander {
    class Lander {
    private:
        Object2D &_object;
        Sprite &_landerSprite;
        Sprite &_engineEffectSprite;
        SpriteAnimation &_engineEffectAnimation;

        Vector2 _thrusterForce = {0.0f, 0.0f};
        Vector2 _thrusterImpulse = {0.0f, 0.0f};

        static void thrusterForceToCenter(Vector2 force, const b2BodyId bodyId);
        static void thrusterImpulseToCenter(Vector2 force, const b2BodyId bodyId);

    public:
        Lander(
            Object2D &object,
            Sprite &landerSprite,
            Sprite &engineEffectSprite,
            SpriteAnimation &engineEffectAnimation
        );

        [[nodiscard]] Object2D &getObject() const;
        void update(const Float dt, const b2BodyId bodyId) const;
        void addForceX(const Float force);
        void addForceY(const Float force);
        void resetForceX();
        void resetForceY();
    };
}

#endif //MOONLANDER_LANDER_H
