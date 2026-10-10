// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_SPRITE_H
#define MOONLANDER_SPRITE_H

#include <Magnum/GL/Texture.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Timeline.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Math/Color.h>

#include "MoonLander.h"

namespace MoonLander {
    using namespace Magnum;

    class Sprite {
    private:
        Shaders::FlatGL2D &_shader;
        GL::Texture2D &_texture;
        GL::Mesh &_mesh;

        Int _frameIndex = Int{0};
        Int _frameCount = Int{0};

        Vector2i _frameSize = Vector2i{16, 16};
        Vector2i _gridSize = Vector2i{1, 1};

    public:
        Sprite(Shaders::FlatGL2D &shader, GL::Texture2D &texture, GL::Mesh &mesh,
               const Vector2i &frameSize);

        void setFrameIndex(Int frameIndex);
        [[nodiscard]] Int getFrameIndex() const;
        [[nodiscard]] Int &getFrameIndexRef();
        [[nodiscard]] Int getFrameCount() const;
        void draw(const Matrix3 &cameraProjectionMatrix, const Matrix3 &objectTransformationMatrix) const;
    };
}

#endif //MOONLANDER_SPRITE_H
