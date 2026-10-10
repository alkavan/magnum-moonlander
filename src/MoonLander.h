#ifndef MOONLANDER_GAME_H
#define MOONLANDER_GAME_H

#include <Magnum/SceneGraph/TranslationRotationScalingTransformation2D.h>

#include <box2d/box2d.h>

namespace MoonLander {
    using namespace Magnum;
    using namespace Math::Literals;

    typedef SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation2D> Object2D;
    typedef SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation2D> Scene2D;

    class GravityConstant {
    public:
        static constexpr auto Earth = b2Vec2{.x = 0.0f, .y = -9.81f};
        static constexpr auto Moon = b2Vec2{.x = 0.0f, .y = -1.62f};
    };
}

#endif //MOONLANDER_GAME_H
