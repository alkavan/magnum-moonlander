#include <Corrade/Utility/Arguments.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/Math/Time.h>
#include <Magnum/Math/Distance.h>

#include <Magnum/Shaders/Flat.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Square.h>

#include "MoonLander/Game.h"
#include "MoonLander/Level.h"
#include "MoonLander/CameraControl.h"
#include "MoonLander/AssetManager.h"
#include "MoonLander/Sprite.h"
#include "MoonLander/SpriteAnimation.h"

#include <version_config.h>

namespace Magnum::Game {
    using namespace Math::Literals;

    static void UpdateZoomByDistance(CameraControl& cameraControl, Vector2 p1, Vector2 p2);

    constexpr Float _engineForceStep = 1.0f;

    Vector2 _previousPointerPosition = {0.0f, 0.0f};
    Float _previousVelocityMagnitude = 0.0f;

    /**
     * Represents the MoonLander application, inheriting from Platform::Application.
     * This class manages the main game logic, rendering, and input handling
     * for the MoonLander game.
     */
    class MoonLander final : public Platform::Application {
    public:
        virtual ~MoonLander() = default;
        explicit MoonLander(const Arguments &arguments);

    private:
        void drawEvent() override;
        void tickEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void pointerMoveEvent(PointerMoveEvent &event) override;
        void scrollEvent(ScrollEvent &event) override;
        void pointerPressEvent(PointerEvent& event) override;

        Scene2D _scene{};
        Timeline _timeline{};

        AssetManager _asset;

        Optional<CameraControl> _cc;
        Optional<b2World> _world;

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
    };

    MoonLander::MoonLander(const Arguments &arguments) : Platform::Application{arguments, NoCreate} {
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
            const auto windowTitle = std::format("Magnum Moonlander ({}-alpha)", PROJECT_VERSION);
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

        // create Box2D world with the usual gravity vector
        _world.emplace(GravityConstant::moon);

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

            auto landerBody = newWorldObjectBody(
                *_world,
                *_landerObject,
                transformation,
                landerScale,
                b2_dynamicBody,
                2.0);

            _landerSprite.emplace(_spriteShader, *landerTexture, _landerSpriteMesh, Vector2i{20, 20});

            // engine effect
            _engineEffectObject.emplace(_landerObject.get());
            _engineEffectObject->translateLocal({0, -1.5});
            _engineEffectObject->setScaling(engineEffectScale);

            _engineEffectSprite.emplace(_spriteShader, *engineEffectTexture, _engineEffectSpriteMesh, Vector2i{8, 8});
            _engineEffectAnimation.emplace(*_engineEffectSprite, 0.1f);

            // lander
            _lander.emplace(*_landerObject, *landerBody, *_landerSprite, *_engineEffectSprite, *_engineEffectAnimation);
        }

        _engineEffectAnimation->start();

        setSwapInterval(1);
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        setMinimalLoopPeriod(16.0_msec);
        _timeline.start();
#endif
    }

    void MoonLander::pointerMoveEvent(PointerMoveEvent &event) {
        // @todo: implement display of coords when pointer moves
    }

    void MoonLander::pointerPressEvent(PointerEvent &event)
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
    void MoonLander::scrollEvent(ScrollEvent &event) {
        _cc->OnScrollEvent(event);
    }

    void MoonLander::keyPressEvent(KeyEvent &event) {
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

    void MoonLander::keyReleaseEvent(KeyEvent &event) {
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

    void MoonLander::drawEvent() {
        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

        _level->draw(_cc->getCamera());

        _landerSprite->draw(
                _cc->getCamera().projectionMatrix(),
                _landerObject->transformationMatrix()
                );

        _engineEffectSprite->draw(
                _cc->getCamera().projectionMatrix(),
                _engineEffectObject->absoluteTransformationMatrix()
                );

        swapBuffers();
        redraw();
    }

    void MoonLander::tickEvent() {
        _timeline.nextFrame();
        const auto dt = _timeline.previousFrameDuration();

        // step the world and update all object positions
        _world->Step(dt, 6, 2);

        // update
        // _cc->updateProjection();

        _engineEffectAnimation->tick();

        _level->update(dt);

        if(_lander) {
            _lander->update(dt);
        }

        // move camera to lander position
        // const auto landerPosition = Vector2{
            // _lander->getBody().GetPosition().x,
            // _lander->getBody().GetPosition().y
        // };

        // _cc->moveTo(landerPosition);

        const Vector2 shipPosition = _lander->getObject().translation();
        const Vector2 screenCenter = Vector2{windowSize()} / 2.0f;
        UpdateZoomByDistance(*_cc, shipPosition, screenCenter);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Handle zooming by lander velocity
        //
        // const Vector2 landerVelocity = _lander->getVelocity();
        //
        // // Stop zooming by velocity when sufficiently close to the anchor
        // if (_cc->isZoomingByVelocity() && _cc->isZoomCloseToAnchor(0.01f)) {
        //     _cc->stopZoomingByVelocity();
        //     // todo: set to anchor.
        // }
        //
        // if (const float velocityMagnitude = std::abs(landerVelocity.y()); velocityMagnitude >= 0.1f) {
        //     if (!_cc->isZoomingByVelocity()) {
        //         _cc->startZoomingByVelocity();
        //     }
        //
        //     // Zoom OUT (reduce zoom) proportional to velocity
        //     const auto zoomAdjustment = Vector2{velocityMagnitude * 0.5f + 0.1f, velocityMagnitude * 0.5f + 0.1f};
        //     _cc->zoomOut(zoomAdjustment);
        // } else {
        //     if (!_cc->isZoomingByVelocity()) {
        //         _cc->startZoomingByVelocity();
        //     }//
        //
        //     constexpr float minimumZoomIncrement = 0.01f;
        //     // Zoom IN (increase zoom) faster when nearly stationary
        //     const float fastZoomAdjustment = std::max((0.1f - velocityMagnitude) * 2.0f, minimumZoomIncrement);
        //     // _cc->zoomIn(Vector2{fastZoomAdjustment, fastZoomAdjustment});
        //     _cc->zoomToAnchor(Vector2{fastZoomAdjustment, fastZoomAdjustment});
        // }
    }

    // Static function to adjust zoom dynamically based on ship positions
    static void UpdateZoomByDistance(CameraControl& cameraControl, const Vector2 p1, const Vector2 p2) {
        // Calculate the Euclidean distance between the two points (p1 and p2)
        const float distance = Math::Distance::pointPoint(p2, p1);
        Debug() << "distance: " << distance;
        // Define zoom factor based on the distance
        // Vector2 targetZoom = cameraControl.getZoomAnchor(); // Default to anchor zoom


        if (distance > 0.0f) {
            cameraControl.zoomToMax(Vector2(distance * 0.001f));
            // targetZoom += Vector2(distance * 0.1f); // Scale zoom based on distance
        }

        // Clamp the target zoom between the minimum and maximum allowed zooms
        // targetZoom = clamp(targetZoom, cameraControl.getZoomMin(), cameraControl.getZoomMax());

        // Set the camera's zoom to the clamped value
        // cameraControl.setZoom(targetZoom);
    }
}

MAGNUM_APPLICATION_MAIN(Magnum::Game::MoonLander)