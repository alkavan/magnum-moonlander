// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include <Corrade/Utility/Arguments.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/Math/Time.h>
#include <Magnum/Math/Distance.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/Animation/Easing.h>

#include <Magnum/Shaders/Flat.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>

#include "MoonLander.h"
#include "Level.h"
#include "CameraControl.h"
#include "AssetManager.h"
#include "Sprite.h"
#include "SpriteAnimation.h"

#include <version_config.h>

namespace {
    using namespace Magnum;
    using namespace Math::Literals;
    using namespace MoonLander;

    // Projection size grows with speed (Box2D m/s). Larger size is zoom-out.
    constexpr Float ZoomSpeedIdle = 0.8f;
    constexpr Float ZoomSpeedFast = 8.0f;
    constexpr Float SpeedSmoothTime = 0.25f;
    constexpr Float ZoomSmoothTime = 0.8f;

    struct ZoomByVelocityState {
        Float smoothedSpeed = 0.0f;
        bool primed = false;
    };

    ZoomByVelocityState _zoomByVelocity{};

    Vector2 zoomForSpeed(const CameraControl& cameraControl, const Float speed) {
        const Float normalized = Math::clamp(
            (speed - ZoomSpeedIdle) / (ZoomSpeedFast - ZoomSpeedIdle),
            0.0f, 1.0f);
        // Flat at both ends, so creeping off the pad and settling do not twitch the lens.
        const Float shaped = Animation::Easing::smootherstep(normalized);
        return Math::lerp(cameraControl.getZoomAnchor(), cameraControl.getZoomMax(), shaped);
    }

    void UpdateZoomByVelocity(CameraControl& cameraControl, const Vector2 velocity, const Float dt, const bool immediate) {
        const Float speed = velocity.length();

        // Game start must already be the speed-matched zoom. Easing out of the
        // constructor default looks like a zoom nobody asked for.
        if(immediate || !_zoomByVelocity.primed) {
            _zoomByVelocity.smoothedSpeed = speed;
            _zoomByVelocity.primed = true;
            cameraControl.setZoom(zoomForSpeed(cameraControl, speed));
            cameraControl.startZoomingByVelocity();
            return;
        }

        // Clamp so a stalled first frame cannot sling the zoom across its range.
        const Float step = Math::clamp(dt, 0.0f, 0.05f);
        const Float speedBlend = 1.0f - Math::exp(-step / SpeedSmoothTime);
        _zoomByVelocity.smoothedSpeed = Math::lerp(_zoomByVelocity.smoothedSpeed, speed, speedBlend);

        const Vector2 target = zoomForSpeed(cameraControl, _zoomByVelocity.smoothedSpeed);
        const Float zoomBlend = 1.0f - Math::exp(-step / ZoomSmoothTime);
        cameraControl.setZoom(Math::lerp(cameraControl.getZoom(), target, zoomBlend));
    }

    constexpr Float _engineForceStep = 1.0f;

    Vector2 _previousPointerPosition = {0.0f, 0.0f};

    /**
     * Represents the MoonLander application, inheriting from Platform::Application.
     * This class manages the main game logic, rendering, and input handling
     * for the MoonLander game.
     */
    class MoonLanderGame final : public Platform::Application {
    public:
        ~MoonLanderGame();
        explicit MoonLanderGame(const Arguments &arguments);

    protected:
        /* Protected in Sdl2Application; narrowing it to private warns. */
        void tickEvent() override;

    private:
        void drawEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void pointerMoveEvent(PointerMoveEvent &event) override;
        void scrollEvent(ScrollEvent &event) override;
        void pointerPressEvent(PointerEvent& event) override;

        Scene2D _scene{};
        Timeline _timeline{};

        AssetManager _asset;

        Optional<CameraControl> _cc;

        Shaders::FlatGL2D _spriteShader{NoCreate};

        Containers::Pointer<Level> _level;
        Containers::Pointer<Lander> _lander;

        Containers::Pointer<Object2D> _landerObject;
        Containers::Pointer<Object2D> _engineEffectObject;

        Containers::Pointer<Sprite> _landerSprite;
        Containers::Pointer<Sprite> _engineEffectSprite;

        GL::Mesh _landerSpriteMesh{NoCreate};
        GL::Mesh _engineEffectSpriteMesh{NoCreate};

        Containers::Pointer<SpriteAnimation> _engineEffectAnimation;

        b2WorldId _worldId{};
        b2BodyId _landerBodyId{};
        Vector2 _cameraPanOffset{};
    };

    MoonLanderGame::~MoonLanderGame() {
        // Clean up Box2D resources
        if (_level) {
            _level.reset(nullptr);
        }

        // Destroy the Box2D world
        b2DestroyWorld(_worldId);
    }

    MoonLanderGame::MoonLanderGame(const Arguments &arguments) : Platform::Application{arguments, NoCreate} {
        Utility::Arguments args;

        // TODO: add command options
//        args.addOption("transformation", "1 0 0 0")
//        .setHelp("transformation", "initial pyramid transformation")
//        .addSkippedPrefix("magnum", "engine-specific options")
//        .parse(arguments.argc, arguments.argv);


        /*
         * try 8x MSAA, fall back to zero samples if not possible.
         * enable only 2x MSAA if we have enough DPI.
         */
        {
            const auto windowTitle = std::format("Magnum MoonMander ({}-alpha)", PROJECT_VERSION);
            const Vector2 dpiScaling = this->dpiScaling({});
            Configuration conf;
            conf.setTitle(windowTitle).setSize(conf.size(), dpiScaling);
            GLConfiguration glConf;
            glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
            if (!tryCreate(conf, glConf))
                create(conf, glConf.setSampleCount(0));
        }

        // setup renderer
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One,
                                       GL::Renderer::BlendFunction::OneMinusSourceAlpha);

        _spriteShader = Shaders::FlatGL2D {Shaders::FlatGL2D::Configuration{}
            .setFlags(Shaders::FlatGL2D::Flag::Textured
                | Shaders::FlatGL2D::Flag::TextureTransformation)};

        // load image textures
        _asset.addTexture("Lander", "Lander.png");
        _asset.addTexture("LanderEngineEffect", "LanderEngineEffect.png");

        // setup camera control
        _cc.emplace(CameraControl{new Object2D{&_scene}});

        // create box2d world with gravity vector
        auto worldDef = b2DefaultWorldDef();
        worldDef.gravity = GravityConstant::Moon;
        _worldId = b2CreateWorld(&worldDef);

        // create and initialize level
        _level.emplace(_scene, _worldId);
        _level->initialize();

        // create mesh for sprites
        _landerSpriteMesh = MeshTools::compile(squareSolid(Primitives::SquareFlag::TextureCoordinates));
        _engineEffectSpriteMesh = MeshTools::compile(squareSolid(Primitives::SquareFlag::TextureCoordinates));

        {
            auto landerTexture = _asset.getTexture("Lander");
            auto engineEffectTexture = _asset.getTexture("LanderEngineEffect");

            Vector2 landerScale = {
                20.f * _cc->getCamera().projectionMatrix().scaling().sum(),
                20.f * _cc->getCamera().projectionMatrix().scaling().sum(),
            };

            Vector2 engineEffectScale = {
                8.f * _cc->getCamera().projectionMatrix().scaling().sum(),
                8.f * _cc->getCamera().projectionMatrix().scaling().sum(),
            };

            auto transformation = DualComplex::translation(Vector2::yAxis(10.0f));

            // lander
            _landerObject.emplace(&_scene);
            _landerObject->setScaling(landerScale);

            _landerBodyId = newWorldObjectBody(
                _worldId,
                _landerObject.get(),
                transformation,
                landerScale,
                b2_dynamicBody,
                2.0
                );

            _landerSprite.emplace(_spriteShader, *landerTexture, _landerSpriteMesh, Vector2i{20, 20});

            // engine effect
            _engineEffectObject.emplace(_landerObject.get());
            _engineEffectObject->translateLocal({0, -1.5});
            _engineEffectObject->setScaling(engineEffectScale);

            _engineEffectSprite.emplace(_spriteShader, *engineEffectTexture, _engineEffectSpriteMesh, Vector2i{8, 8});
            _engineEffectAnimation.emplace(*_engineEffectSprite, 0.1f);

            // lander
            _lander.emplace(*_landerObject, *_landerSprite, *_engineEffectSprite, *_engineEffectAnimation);

            // Body is already at this translation; keep the first frame on it.
            _cc->moveTo(transformation.translation());

            // Velocity is still zero, so this snaps to the idle zoom. Do this after
            // the lander scale is captured from the projection.
            const auto velocity = b2Body_GetLinearVelocity(_landerBodyId);
            UpdateZoomByVelocity(*_cc, {velocity.x, velocity.y}, 0.0f, true);
        }

        _engineEffectAnimation->start();

        setSwapInterval(1);
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        setMinimalLoopPeriod(16.0_msec);
#endif
        _timeline.start();
    }

    void MoonLanderGame::pointerMoveEvent(PointerMoveEvent &event) {
        if(!event.isPrimary() ||
            !(event.pointers() & (Pointer::MouseLeft|Pointer::Finger)))
            return;

        // Window pixels, Y down, into Box2D units. projectionSize() already
        // includes the aspect policy, which is what click-to-spawn uses.
        const Vector2 worldDelta = event.relativePosition()/Vector2{windowSize()}*
            _cc->getCamera().projectionSize()*Vector2::yScale(-1.0f);
        if(worldDelta.dot() == 0.0f)
            return;

        // Grab the world. Stored because tickEvent recenters on the lander.
        _cameraPanOffset -= worldDelta;
        _cc->move(-worldDelta);
        event.setAccepted(true);
    }

    void MoonLanderGame::pointerPressEvent(PointerEvent &event)
    {
        if(event.isPrimary())
        {
            _previousPointerPosition = event.position();

            if(event.pointer() & (Pointer::MouseLeft|Pointer::Finger))
            {
                const auto position = _cc->projectedPosition(
                    Vector2{event.position()},
                    Vector2{windowSize()}
                    );

                const auto transformation = DualComplex::translation(position + _cc->getContainerTranslation());
                _level->addBox(transformation);
            }
        }

        event.setAccepted(true);
    }

    /**
     *
     * @param event Application mouse scroll event.
     */
    void MoonLanderGame::scrollEvent(ScrollEvent &event) {
        _cc->OnScrollEvent(event);
    }

    void MoonLanderGame::keyPressEvent(KeyEvent &event) {
        // pass key event to camera control
        _cc->OnKeyPressEvent(event);

        // exit game
        if(event.key() == Key::Esc) {
            event.setAccepted(true);
            exit();
        }

        // forward
        if(event.key() == Key::W) {
            _lander->addForceY(_engineForceStep);
            event.setAccepted(true);
        }

        // backward
        if(event.key() == Key::S) {
            _lander->addForceY(-_engineForceStep);
            event.setAccepted(true);
        }

        // right
        if(event.key() == Key::D) {
            _lander->addForceX(_engineForceStep);
            event.setAccepted(true);
        }

        // left
        if(event.key() == Key::A) {
            _lander->addForceX(-_engineForceStep);
            event.setAccepted(true);
        }

        if( ! event.isAccepted()) {
            Debug{} << "unhandled key press: " << event.keyName();
            event.setAccepted(true);
        }
    }

    void MoonLanderGame::keyReleaseEvent(KeyEvent &event) {
        if(event.key() == Key::W) {
            _lander->resetForceY();
            event.setAccepted(true);
        }

        if(event.key() == Key::S) {
            _lander->resetForceY();
            event.setAccepted(true);
        }

        if(event.key() == Key::D) {
            _lander->resetForceX();
            event.setAccepted(true);
        }

        if(event.key() == Key::A) {
            _lander->resetForceX();
            event.setAccepted(true);
        }

        if( ! event.isAccepted()) {
            Debug{} << "unhandled key release: " << event.keyName();
            event.setAccepted(true);
        }
    }

    void MoonLanderGame::drawEvent() {
        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

        _level->draw(_cc->getCamera());

        // camera.draw() uses projection * cameraMatrix * model. Sprites are not in that
        // group, so they must apply the same view or they drift off the Box2D bodies.
        const Matrix3 cameraMatrix = _cc->getCamera().cameraMatrix();

        _landerSprite->draw(
                _cc->getCamera().projectionMatrix(),
                cameraMatrix * _landerObject->absoluteTransformationMatrix()
                );

        _engineEffectSprite->draw(
                _cc->getCamera().projectionMatrix(),
                cameraMatrix * _engineEffectObject->absoluteTransformationMatrix()
                );

        swapBuffers();
        redraw();
    }

    void MoonLanderGame::tickEvent()
    {
        _timeline.nextFrame();
        const auto dt = _timeline.previousFrameDuration();

        // step the world and update all object positions
        b2World_Step(_worldId, dt, 6);

        _engineEffectAnimation->tick();

        _level->update(dt);

        if(_lander) {
            _lander->update(dt, _landerBodyId);

            const auto [_x, _y] = b2Body_GetPosition(_landerBodyId);
            const Vector2 landerPosition{_x, _y};

            // Move the camera in world space. Do not subtract this from body positions.
            _cc->moveTo(landerPosition + _cameraPanOffset);
            const auto velocity = b2Body_GetLinearVelocity(_landerBodyId);
            UpdateZoomByVelocity(*_cc, {velocity.x, velocity.y}, dt, false);
        }

        // update
        _cc->updateProjection();
    }
}

MAGNUM_APPLICATION_MAIN(MoonLanderGame)
