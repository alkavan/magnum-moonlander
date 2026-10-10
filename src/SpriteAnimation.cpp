// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "SpriteAnimation.h"

namespace MoonLander {

SpriteAnimation::SpriteAnimation(Sprite &sprite, const Float frameDelay) : _sprite(sprite) {
    Containers::Array<std::pair<Float, Int> > frames{
        ValueInit, static_cast<std::size_t>(_sprite.getFrameCount())
    };

    Float frameTime = 0.0f;
    for (std::size_t i = 0; i != frames.size(); ++i) {
        frames[i] = {frameTime, i};
        frameTime += frameDelay;
    }
    _frames = Animation::Track{std::move(frames), Math::select};

    _player.addWithCallbackOnChange(
        _frames,
        [](Float, const Int &frame, SpriteAnimation &animation) {
            animation._sprite.setFrameIndex(frame);
        }, _sprite.getFrameIndexRef(), *this);
}

void SpriteAnimation::start() {
    _timeline.start();
    _player
            .setPlayCount(0)
            .play(_timeline.previousFrameTime());
}

void SpriteAnimation::pause() {
    _player.pause(_timeline.previousFrameTime());
}

void SpriteAnimation::tick() {
    _player.advance(_timeline.previousFrameTime());
    _timeline.nextFrame();
}

} // namespace MoonLander
