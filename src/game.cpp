#include <Corrade/Utility/Arguments.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <Magnum/Shaders/Flat.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Square.h>

#include "MoonLander/Game.h"
#include "MoonLander/Level.h"
#include "MoonLander/CameraControl.h"
#include "MoonLander/AssetManager.h"
#include "MoonLander/Sprite.h"
#include "MoonLander/SpriteAnimation.h"

namespace Magnum::Game {
    using namespace Math::Literals;

    class MoonLander final : public Platform::Application {
    public:
        virtual ~MoonLander() = default;
        explicit MoonLander(const Arguments &arguments);

    private:
        void drawEvent() override;
        void tickEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void mousePressEvent(MouseEvent &event) override;
        void mouseScrollEvent(MouseScrollEvent &event) override;

        Scene2D _scene{};
        Timeline _timeline{};

        AssetManager _asset;

        Optional<CameraControl> _cc;
        Optional<b2World> _world;

        Shaders::Flat2D _spriteShader{NoCreate};
        GL::Mesh _spriteMesh{NoCreate};

        Level *_level;

        Containers::Pointer<Lander> _lander;

        Containers::Pointer<Object2D> _landerObject;
        Containers::Pointer<Object2D> _engineEffectObject;

        Containers::Pointer<Sprite> _landerSprite;
        Containers::Pointer<Sprite> _engineEffectSprite;

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
            const Vector2 dpiScaling = this->dpiScaling({});
            Configuration conf;
            conf.setTitle("Magnum Moonlander (1.0-alpha)")
                    .setSize(conf.size(), dpiScaling);
            GLConfiguration glConf;
            glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
            if (!tryCreate(conf, glConf))
                create(conf, glConf.setSampleCount(0));
        }

        // setup renderer
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One,
                                       GL::Renderer::BlendFunction::OneMinusSourceAlpha);

        // initialize shader for sprites
        _spriteShader = Shaders::Flat2D {
                Shaders::Flat2D::Flag::Textured |
                Shaders::Flat2D::Flag::TextureTransformation
        };

        // load image textures
        _asset.addTexture("Lander", "Lander.png");
        _asset.addTexture("LanderEngineEffect", "LanderEngineEffect.png");

        // setup camera control
        _cc.emplace(CameraControl{new Object2D{&_scene}});

        // create Box2D world with the usual gravity vector
        _world.emplace(GravityConstant::moon);

        // create and initialize level
        _level = new Level(_scene, *_world);
        _level->initialize();

        // create mesh for sprites
        _spriteMesh = MeshTools::compile(Primitives::squareSolid(Primitives::SquareFlag::TextureCoordinates));

        auto landerTexture = _asset.getTexture("Lander");
        auto engineEffectTexture = _asset.getTexture("LanderEngineEffect");

        Vector2 landerScale = {
                Float(20) * _cc->getCamera().projectionMatrix().scaling().sum(),
                Float(20) * _cc->getCamera().projectionMatrix().scaling().sum(),
        };

        Vector2 engineEffectScale = {
                Float(8) * _cc->getCamera().projectionMatrix().scaling().sum(),
                Float(8) * _cc->getCamera().projectionMatrix().scaling().sum(),
        };

        {
            auto transformation = DualComplex::translation(Vector2::yAxis(10.0f));
            // lander
            _landerObject.emplace(&_scene);
            _landerObject->setScaling(landerScale);
            auto landerBody = Game::newWorldObjectBody(*_world, *_landerObject, transformation, landerScale, b2_dynamicBody, 2.0);
            _landerSprite.emplace(_spriteShader, *landerTexture, _spriteMesh, Vector2i{20, 20});

            // engine effect
            _engineEffectObject.emplace(_landerObject.get());
            _engineEffectObject->translateLocal({0, -1.5});
            _engineEffectObject->setScaling(engineEffectScale);

            _engineEffectSprite.emplace(_spriteShader, *engineEffectTexture, _spriteMesh, Vector2i{8, 8});

            _engineEffectAnimation.emplace(*_engineEffectSprite, 0.1f);

            // lander
            _lander.emplace(*_landerObject, *landerBody, *_landerSprite, *_engineEffectSprite, *_engineEffectAnimation);
        }

        _engineEffectAnimation->start();

        setSwapInterval(1);
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        setMinimalLoopPeriod(16);
        _timeline.start();
#endif
    }

    void MoonLander::mousePressEvent(MouseEvent &event) {
        if (event.button() != MouseEvent::Button::Left) return;

        const auto position = _cc->projectedPosition(
                Vector2{event.position()},
                Vector2{windowSize()}
                );

        auto transformation = DualComplex::translation(position + _cc->getContainerTranslation());

        _level->addBox(transformation);
    }

    void MoonLander::mouseScrollEvent(MouseScrollEvent &event) {
        // zoom-in
        if(event.offset().y() > 0) {
            _cc->zoomIn();
        }

        // zoom-out
        if(event.offset().y() < 0) {
            _cc->zoomOut();
        }
    }

    void MoonLander::keyPressEvent(Platform::Sdl2Application::KeyEvent &event) {
//        auto screenProjection = Matrix3::projection(Vector2{windowSize()});

        // move camera to the left
//        if(event.key() == KeyEvent::Key::Left) {
//            _cameraControl->move(Vector2::xAxis(-5.0f) * screenProjection.scaling());
//        }

//        if(event.key() == KeyEvent::Key::Right) {
//            _cameraControl->move(Vector2::xAxis(5.0f) * screenProjection.scaling());
//        }

        // exit game
        if(event.key() == KeyEvent::Key::Esc) {
            event.setAccepted(true);
            exit();
        }

        auto forceStep = Float(1.0);

        if(event.key() == KeyEvent::Key::W) {
            _lander->addForceY(forceStep);
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::S) {
            _lander->addForceY(-forceStep);
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::D) {
            _lander->addForceX(forceStep);
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::A) {
            _lander->addForceX(-forceStep);
            event.setAccepted(true);
        }

        if( ! event.isAccepted()) {
            Debug{} << "unhandled key press: " << event.keyName().c_str();
            event.setAccepted(true);
        }
    }

    void MoonLander::keyReleaseEvent(KeyEvent &event) {
        if(event.key() == KeyEvent::Key::W) {
            _lander->resetForceY();
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::S) {
            _lander->resetForceY();
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::D) {
            _lander->resetForceX();
            event.setAccepted(true);
        }

        if(event.key() == KeyEvent::Key::A) {
            _lander->resetForceX();
            event.setAccepted(true);
        }

        if( ! event.isAccepted()) {
            Debug{} << "unhandled key release: " << event.keyName().c_str();
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
        _cc->updateProjection();

        _engineEffectAnimation->tick();

        _level->update(dt);

        if(_lander) {
            _lander->update(dt);
        }

        // move camera to lander position
//        auto landerPosition = Vector2{
//            _lander->getBody().GetPosition().x,
//            _lander->getBody().GetPosition().y
//        };
//
//        _cc->moveTo(landerPosition);

        // TODO: zoom out and in when velocity between 150.0 to 50.0
//        auto landerVelocity = _lander->getVelocity();
//        if(landerVelocity > Vector2{1.0f, 1.0f}) {
//            _cameraControl->zoomAdd({1.0f, 1.0f});
//        }
    }

}

MAGNUM_APPLICATION_MAIN(Magnum::Game::MoonLander)