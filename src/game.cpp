#include <Corrade/Utility/Arguments.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <Magnum/Shaders/FlatGL.h>
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

    class MoonLander : public Platform::Application {
    public:
        explicit MoonLander(const Arguments &arguments);

    private:
        void drawEvent() override;
        void tickEvent() override;

        void keyPressEvent(KeyEvent &event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void mousePressEvent(MouseEvent &event) override;
        void mouseScrollEvent(MouseScrollEvent &event) override;

        Scene2D _scene{};
        Timeline _timeline;

        AssetManager _asset;
        Level *_level;

        Containers::Optional<CameraControl> _cc;
        Containers::Optional<b2World> _world;

        Shaders::FlatGL2D _spriteShader{NoCreate};
        GL::Mesh _spriteMesh{NoCreate};

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

        _spriteShader = Shaders::FlatGL2D {
                Shaders::FlatGL2D::Flag::Textured |
                Shaders::FlatGL2D::Flag::TextureTransformation
        };

        _spriteMesh = MeshTools::compile(Primitives::squareSolid(Primitives::SquareFlag::TextureCoordinates));

        // setup camera control
        _cc.emplace(CameraControl{new Object2D{&_scene}});

        // create Box2D world with the usual gravity vector
        _world.emplace(GravityConstant::moon);

        // create and initialize level
        _level = new Level(_scene, *_world);
        _level->initialize();

        // load images as textures
        _asset.addTexture("Lander", "Lander.png");
        _asset.addTexture("LanderEngineEffect", "LanderEngineEffect.png");

        auto landerTexture = _asset.getTexture("Lander");
        auto engineEffectTexture = _asset.getTexture("LanderEngineEffect");

        Vector2i landerImagSize =  landerTexture->imageSize(0);

        Vector2 landerScale = {
                Float(landerImagSize.x()) * _cc->getCamera().projectionMatrix().scaling().sum(),
                Float(landerImagSize.y()) * _cc->getCamera().projectionMatrix().scaling().sum(),
        };

        auto transformation = DualComplex::translation(Vector2::yAxis(10.0f));

        {   // lander
            _landerObject.emplace(&_scene);
            auto landerBody = Game::newWorldObjectBody(*_world, *_landerObject, transformation, landerScale, b2_dynamicBody, 2.0);

            auto landerSprite = new Sprite{_spriteShader, *landerTexture, _spriteMesh, {20, 20}};
            _landerSprite.emplace(*landerSprite);

            // engine effect
            _engineEffectObject.emplace(_landerObject.get());
            _engineEffectObject->translate({0, -0.5});

            auto engineEffectSprite = new Sprite{_spriteShader, *engineEffectTexture, _spriteMesh, {8, 8}};
            _engineEffectSprite.emplace(*engineEffectSprite);

            _engineEffectAnimation.emplace(*_engineEffectSprite, 0.1f);

            // lander
            _lander.emplace(Lander{*_landerObject, *landerBody, *_landerSprite, *_engineEffectSprite, *_engineEffectAnimation});
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

    void MoonLander::mouseScrollEvent(Platform::Sdl2Application::MouseScrollEvent &event) {
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
            Debug{} << "unhandled key press: " << event.keyName();
            event.setAccepted(true);
        }
    }

    void MoonLander::keyReleaseEvent(Platform::Sdl2Application::KeyEvent &event) {
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
            Debug{} << "unhandled key release: " << event.keyName();
            event.setAccepted(true);
        }
    }

    void MoonLander::drawEvent() {
        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

        _landerSprite->draw(
                _cc->getCamera().projectionMatrix(),
                _landerObject->transformationMatrix()
                );

        _engineEffectSprite->draw(
                _cc->getCamera().projectionMatrix(),
                _engineEffectObject->transformationMatrix()
                );

        _level->draw(_cc->getCamera());

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