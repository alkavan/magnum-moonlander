#ifndef MAGNUM_MOONLANDER_GAME_H
#define MAGNUM_MOONLANDER_GAME_H

#include <Magnum/SceneGraph/TranslationRotationScalingTransformation2D.h>

#include <box2d/box2d.h>

namespace Magnum::Game {
    typedef SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation2D> Object2D;
    typedef SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation2D> Scene2D;

    class GravityConstant {
    public:
        static inline const b2Vec2 earth = b2Vec2{0.0f, -9.81f};
        static inline const b2Vec2 moon = b2Vec2{0.0f, -1.62f};
    };
}

#endif //MAGNUM_MOONLANDER_GAME_H
