// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include <Corrade/Utility/Arguments.h>
#include <Corrade/Containers/EnumSet.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/Math/Time.h>
#include <Magnum/Math/Functions.h>

#include <Magnum/Shaders/Flat.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>

#include "MoonLander.h"
#include "Level.h"
#include "World.h"
#include "CameraControl.h"
#include "AssetManager.h"
#include "Sprite.h"
#include "SpriteAnimation.h"

#include <version_config.h>

namespace {
    using namespace Magnum;
    using namespace Math::Literals;
    using namespace MoonLander;

    constexpr Float _engineForceStep = 1.0f;

    enum class DirectionKey: UnsignedByte {
        Up = 1 << 0,
        Down = 1 << 1,
        Right = 1 << 2,
        Left = 1 << 3
    };
    using DirectionKeys = Containers::EnumSet<DirectionKey>;
    CORRADE_ENUMSET_OPERATORS(DirectionKeys)

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

        Containers::Pointer<World> _world;
        b2BodyId _landerBodyId{};
        Vector2 _cameraPanOffset{};
        DirectionKeys _pressedKeys{};

        void applyPressedThrust() {
            _lander->resetForceX();
            _lander->resetForceY();
            if(_pressedKeys & DirectionKey::Up)
                _lander->addForceY(_engineForceStep);
            if(_pressedKeys & DirectionKey::Down)
                _lander->addForceY(-_engineForceStep);
            if(_pressedKeys & DirectionKey::Right)
                _lander->addForceX(_engineForceStep);
            if(_pressedKeys & DirectionKey::Left)
                _lander->addForceX(-_engineForceStep);
        }
    };

    MoonLanderGame::~MoonLanderGame() {
        // Clean up Box2D resources (World destructor destroys the Box2D world)
        if (_level) {
            _level.reset(nullptr);
        }
        _world.reset();
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

        // create physics world
        _world.emplace(GravityConstant::Moon);

        // create and initialize level
        _level.emplace(_scene, *_world);
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

            _landerBodyId = _world->createBox(
                _landerObject.get(),
                transformation,
                landerScale,
                b2_dynamicBody,
                2.0f
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
            const auto [_vx, _vy] = b2Body_GetLinearVelocity(_landerBodyId);
            _cc->updateZoomByVelocity({_vx, _vy}, 0.0f, false, true);
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
            !(event.pointers() & (Pointer::MouseLeft|Pointer::Finger)) ||
            !(event.modifiers() & Modifier::Alt)) {
            return;
        }

        // Window pixels, Y down, into Box2D units. projectionSize() already
        // includes the aspect policy, which is what click-to-spawn uses.
        const Vector2 worldDelta = event.relativePosition()/Vector2{windowSize()}*
            _cc->getCamera().projectionSize()*Vector2::yScale(-1.0f);
        if(worldDelta.dot() == 0.0f) {
            return;
        }

        // Grab the world. Stored because tickEvent re-centers on the lander.
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
                // Alt+drag grabs the world
                if(event.modifiers() & Modifier::Alt) {
                    event.setAccepted(true);
                    return;
                }

                // Left click spans a box
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

        bool handled = false;
        if(event.key() == Key::W) { _pressedKeys |= DirectionKey::Up; handled = true; }
        else if(event.key() == Key::S) { _pressedKeys |= DirectionKey::Down; handled = true; }
        else if(event.key() == Key::D) { _pressedKeys |= DirectionKey::Right; handled = true; }
        else if(event.key() == Key::A) { _pressedKeys |= DirectionKey::Left; handled = true; }

        if(handled) {
            applyPressedThrust();
            event.setAccepted(true);
        }

        if( ! event.isAccepted()) {
            Debug{} << "unhandled key press: " << event.keyName();
            event.setAccepted(true);
        }
    }

    void MoonLanderGame::keyReleaseEvent(KeyEvent &event) {
        bool handled = false;
        if(event.key() == Key::W) { _pressedKeys &= ~DirectionKey::Up; handled = true; }
        else if(event.key() == Key::S) { _pressedKeys &= ~DirectionKey::Down; handled = true; }
        else if(event.key() == Key::D) { _pressedKeys &= ~DirectionKey::Right; handled = true; }
        else if(event.key() == Key::A) { _pressedKeys &= ~DirectionKey::Left; handled = true; }

        if(handled) {
            applyPressedThrust();
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
        _world->step(dt, 6);

        _engineEffectAnimation->tick();

        _level->update(dt);

        if(_lander) {
            _lander->update(dt, _landerBodyId);

            const auto [_x, _y] = b2Body_GetPosition(_landerBodyId);
            const Vector2 landerPosition{_x, _y};

            // Move the camera in world space. Do not subtract this from body positions.
            _cc->moveTo(landerPosition + _cameraPanOffset);
            const auto [_vx, _vy] = b2Body_GetLinearVelocity(_landerBodyId);
            _cc->updateZoomByVelocity({_vx, _vy}, dt, static_cast<bool>(_pressedKeys));
        }

        // update
        _cc->updateProjection();
    }
}

MAGNUM_APPLICATION_MAIN(MoonLanderGame)
