#ifndef MAGNUM_MOONLANDER_SPRITEANIMATION_H
#define MAGNUM_MOONLANDER_SPRITEANIMATION_H

#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Timeline.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Functions.h>

#include "Sprite.h"

namespace Magnum::Game {
        class SpriteAnimation {
        public:
            SpriteAnimation(Sprite &sprite, Float frameDelay) :
            _sprite(sprite) {
                Containers::Array<std::pair<Float, Int>> frames{std::size_t(_sprite.getFrameCount())};

                Float frameTime = 0.0f;
                for(std::size_t i = 0; i != frames.size(); ++i) {
                    frames[i] = {frameTime, i};
                    frameTime += frameDelay;
                }
                _frames = Animation::Track<Float, Int>{std::move(frames), Math::select};

                _player.addWithCallbackOnChange(
                        _frames,
                        [](Float, const Int& frame, SpriteAnimation& animation) {
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

#endif //MAGNUM_MOONLANDER_SPRITEANIMATION_H
