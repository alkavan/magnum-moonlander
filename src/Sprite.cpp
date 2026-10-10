// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Sprite.h"

#include <Corrade/Utility/Debug.h>

namespace MoonLander {

Sprite::Sprite(Shaders::FlatGL2D &shader, GL::Texture2D &texture, GL::Mesh &mesh,
               const Vector2i &frameSize) : _shader(shader), _texture(texture), _mesh(mesh), _frameSize(frameSize) {
    auto imageSize = _texture.imageSize(0);

    _gridSize.x() = imageSize.x() / _frameSize.x();
    _gridSize.y() = imageSize.y() / _frameSize.y();
    _frameCount = _gridSize.product();
}

void Sprite::setFrameIndex(Int frameIndex) {
    if (const auto maxFrameIndex = _frameCount - 1; frameIndex > maxFrameIndex) {
        Error() << "unable to set frame index" << frameIndex
                << "last frame is" << maxFrameIndex << "(fallback to 0)";
        frameIndex = 0;
    }

    _frameIndex = frameIndex;
}

Int Sprite::getFrameIndex() const {
    return _frameIndex;
}

Int &Sprite::getFrameIndexRef() {
    return _frameIndex;
}

Int Sprite::getFrameCount() const {
    return _frameCount;
}

void Sprite::draw(const Matrix3 &cameraProjectionMatrix, const Matrix3 &objectTransformationMatrix) const {
    _shader.setTransformationProjectionMatrix(cameraProjectionMatrix * objectTransformationMatrix);

    _shader.setTextureMatrix(
        Matrix3::scaling(1.0f / Vector2{_gridSize}) *
        Matrix3::translation(Vector2{Vector2i{_frameIndex, 0}}));

    _shader.bindTexture(_texture).draw(_mesh);
}

} // namespace MoonLander
