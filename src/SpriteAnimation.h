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

namespace MoonLander {
    class SpriteAnimation {
    public:
        SpriteAnimation(Sprite &sprite, const Float frameDelay) : _sprite(sprite) {
            Containers::Array<std::pair<Float, Int> > frames{static_cast<std::size_t>(_sprite.getFrameCount())};

            Float frameTime = 0.0f;
            for (std::size_t i = 0; i != frames.size(); ++i) {
                frames[i] = {frameTime, i};
                frameTime += frameDelay;
            }
            _frames = Animation::Track{std::move(frames), Math::select};

            _player.addWithCallbackOnChange(
                _frames,
                [](Float, const Int &frame, SpriteAnimation &animation) {
                    //                            Debug{} << "frame: " << frame;
                    animation._sprite.setFrameIndex(frame);
                }, _sprite.getFrameIndexRef(), *this);
        }

        void start() {
            _timeline.start();
            _player
                    .setPlayCount(0)
                    .play(_timeline.previousFrameTime());
        }

        void pause() {
            _player.pause(_timeline.previousFrameTime());
        }

        void tick() {
            _player.advance(_timeline.previousFrameTime());
            _timeline.nextFrame();
        }

    private:
        Sprite &_sprite;

        Animation::Track<Float, Int> _frames;
        Animation::Player<Float> _player;
        Timeline _timeline;
    };
} // Game

#endif //MOONLANDER_SPRITEANIMATION_H
