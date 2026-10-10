// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_SPRITEANIMATION_H
#define MOONLANDER_SPRITEANIMATION_H

#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Timeline.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Functions.h>

#include "Sprite.h"
#include "MoonLander.h"

namespace MoonLander {
    using namespace Magnum;
    using namespace Corrade;

    class SpriteAnimation {
    public:
        SpriteAnimation(Sprite &sprite, const Float frameDelay);

        void start();
        void pause();
        void tick();

    private:
        Sprite &_sprite;

        Animation::Track<Float, Int> _frames;
        Animation::Player<Float> _player;
        Timeline _timeline;
    };
}

#endif //MOONLANDER_SPRITEANIMATION_H
