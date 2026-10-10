// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_BOX_H
#define MOONLANDER_BOX_H

#include "MoonLander.h"

namespace MoonLander {
        class Box {
        private:
            Object2D &_object;
            b2BodyId _bodyId;
            DrawableMesh &_drawable;
        public:
            Box(Object2D &object, const b2BodyId bodyId, DrawableMesh &drawable) :
            _object(object), _bodyId(bodyId), _drawable(drawable) {}

            void update(const Float dt) const {
                const auto userData = b2Body_GetUserData(_bodyId);
                const auto [x, y] = b2Body_GetPosition(_bodyId);
                const auto angle = b2Rot_GetAngle(b2Body_GetRotation(_bodyId));

                static_cast<Object2D*>(userData)->setTranslation({x, y})
                    .setRotation(Complex::rotation(Rad(angle)));
            }
        };
} // Game

#endif //MOONLANDER_BOX_H
