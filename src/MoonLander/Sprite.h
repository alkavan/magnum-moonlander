#ifndef MAGNUM_MOONLANDER_SPRITE_H
#define MAGNUM_MOONLANDER_SPRITE_H

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Sampler.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/ImageView.h>
#include <Magnum/Timeline.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>
#include <Magnum/Trade/MeshData2D.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Math/Color.h>

namespace Magnum::Game {

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
        Sprite(Shaders::FlatGL2D &shader, GL::Texture2D &texture, GL::Mesh &mesh, const Vector2i &frameSize):
        _shader(shader), _texture(texture), _mesh(mesh), _frameSize(frameSize) {


            auto imageSize = _texture.imageSize(0);

            _gridSize.x() = imageSize.x() / _frameSize.x();
            _gridSize.y() = imageSize.y() / _frameSize.y();
            _frameCount = _gridSize.product();
        }

        void setFrameIndex(Int frameIndex) {

            auto maxFrameIndex = _frameCount-1;
            if( frameIndex > maxFrameIndex ) {
                Utility::Error() << "unable to set frame index" << frameIndex
                << "last frame is" << maxFrameIndex << "(fallback to 0)";
                frameIndex = 0;
            }

            _frameIndex = frameIndex;
        }

        [[nodiscard]] Int getFrameIndex() const {
            return _frameIndex;
        }

        [[nodiscard]]  Int& getFrameIndexRef() {
            return _frameIndex;
        }

        [[nodiscard]] Int getFrameCount() const {
            return _frameCount;
        }

        void draw(const Matrix3 &cameraProjectionMatrix, const Matrix3 &objectTransformationMatrix) {

            _shader.setTransformationProjectionMatrix(cameraProjectionMatrix*objectTransformationMatrix);


            _shader.setTextureMatrix(
                    Matrix3::scaling(1.0f/Vector2{_gridSize})*
                    Matrix3::translation(Vector2{Vector2i{_frameIndex, 0}}));

            _shader.bindTexture(_texture).draw(_mesh);
        }
    };
}


#endif //MAGNUM_MOONLANDER_SPRITE_H
