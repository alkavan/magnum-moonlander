#ifndef MAGNUM_MOONLANDER_CAMERACONTROL_H
#define MAGNUM_MOONLANDER_CAMERACONTROL_H

#include <Magnum/SceneGraph/Camera.h>

#include "Game.h"

namespace Magnum::Game {
    class CameraControl {
    public:
        CameraControl(Object2D *cameraObject);

        SceneGraph::Camera2D &getCamera() const {
            return *_camera;
        };

        const Object2D &getCameraObject() const {
            return *_cameraObject;
        };

        Vector2 getContainerTranslation() const;
        Vector2 projectedPosition(Vector2 screenPosition, Vector2 screenSize) const;

        void move(Vector2 displacement);
        void moveTo(Vector2 point);

        void zoomAdd(Vector2 zoomChange);
        void zoomIn();
        void zoomOut();
        void updateProjection();

    private:
        SceneGraph::Camera2D *_camera;
        Object2D *_cameraObject;

        Vector2 _zoom = {50.0f, 50.0f};  // Default zoom

        const Vector2 _zoomMax = {100.0f, 100.0f};
        const Vector2 _zoomMin = {5.0f, 5.0f};
        const Vector2 _zoomDelta = {5.0f, 5.0f};
    };

    CameraControl::CameraControl(Object2D *cameraObject): _cameraObject(cameraObject) {
        auto camera = new SceneGraph::Camera2D{*_cameraObject};

        camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
                .setViewport(GL::defaultFramebuffer.viewport().size());

        camera->setProjectionMatrix(Matrix3::projection(_zoom));

        _camera = camera;
    }

    void CameraControl::zoomAdd(Vector2 zoomChange) {
        _zoom += zoomChange;
    }

    void CameraControl::zoomIn() {
        // Zoom-in
        if((_zoom + _zoomDelta) <= _zoomMax) {
            _zoom += _zoomDelta;
            Debug{} << "ZoomIn: " << _zoom;
        }
        else {
            Debug{} << "[!] MaxZoom: " << _zoomMax;
        }
    }

    void CameraControl::zoomOut() {
        // Zoom-out
        if((_zoom - _zoomDelta) >= _zoomMin) {
            _zoom -= _zoomDelta;
            Debug{} << "ZoomOut: " << _zoom;
        } else {
            Debug{} << "[!] MinZoom: " << _zoomMin;
        }
    }

    void CameraControl::move(Vector2 displacement) {
        _cameraObject->translate(displacement * _zoom);
    }

    void CameraControl::moveTo(Vector2 point) {
        auto positionDiff = point - _cameraObject->translation();
        _cameraObject->translate(positionDiff);
    }

    Vector2 CameraControl::projectedPosition(Vector2 screenPosition, Vector2 screenSize) const {
        return _camera->projectionSize() * Vector2::yScale(-1.0f) *
                              (screenPosition / screenSize - Vector2{0.5f});
    }

    Vector2 CameraControl::getContainerTranslation() const {
        return _cameraObject->translation();
    }

    void CameraControl::updateProjection() {
        _camera->setProjectionMatrix(Matrix3::projection(_zoom));
    }
}
#endif //MAGNUM_MOONLANDER_CAMERACONTROL_H
