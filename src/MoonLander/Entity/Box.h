#ifndef MAGNUM_MOONLANDER_BOX_H
#define MAGNUM_MOONLANDER_BOX_H

#include <box2d/b2_body.h>
#include "../Game.h"

namespace Magnum::Game {
        class Box {
        private:
            Object2D &_object;
            b2Body &_body;
            DrawableMesh &_drawable;
        public:
            Box(Object2D &object, b2Body &body, DrawableMesh &drawable) :
            _object(object), _body(body), _drawable(drawable) {}

            void update(const Float dt) {
                (*reinterpret_cast<Object2D*>(_body.GetUserData().pointer))
                        .setTranslation({_body.GetPosition().x, _body.GetPosition().y})
                        .setRotation(Complex::rotation(Rad(_body.GetAngle())));
            }
        };
} // Game

#endif //MAGNUM_MOONLANDER_BOX_H
