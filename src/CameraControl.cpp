// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "CameraControl.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Math/Matrix3.h>
#include <Corrade/Utility/Debug.h>

namespace MoonLander {

CameraControl::CameraControl(Object2D *cameraObject): _cameraObject(cameraObject), _zoom(50.0f) {
    const auto camera = new SceneGraph::Camera2D{*_cameraObject};

    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setViewport(GL::defaultFramebuffer.viewport().size());

    camera->setProjectionMatrix(Matrix3::projection(_zoom));

    _camera = camera;
}

SceneGraph::Camera2D &CameraControl::getCamera() const {
    return *_camera;
}

const Object2D &CameraControl::getCameraObject() const {
    return *_cameraObject;
}

Vector2 CameraControl::getContainerTranslation() const {
    return _cameraObject->translation();
}

Vector2 CameraControl::projectedPosition(const Vector2 screenPosition, const Vector2 screenSize) const {
    return _camera->projectionSize() * Vector2::yScale(-1.0f) *
                          (screenPosition / screenSize - Vector2{0.5f});
}

void CameraControl::move(const Vector2 displacement) const {
    // World units, same space as Box2D. Multiplying by _zoom (projection size)
    // makes the view slide away from the bodies as soon as you zoom.
    _cameraObject->translate(displacement);
}

void CameraControl::moveTo(const Vector2 point) const {
    const auto positionDiff = point - _cameraObject->translation();
    _cameraObject->translate(positionDiff);
}

Vector2 CameraControl::zoomAdd(const Vector2 zoomDiff) {
    _zoom += zoomDiff;
    if(_zoom > _zoomMax) {
        _zoom = _zoomMax;
    }
    return _zoom;
}

Vector2 CameraControl::zoomSub(const Vector2 zoomDiff) {
    _zoom -= zoomDiff;
    if(_zoom < _zoomMin) {
        _zoom = _zoomMin;
    }
    return _zoom;
}

void CameraControl::zoomIn(const Vector2 zoomDiff) {
    zoomSub(zoomDiff);
    if (_zoom <= _zoomMin) {
        Debug{} << "[!] (CameraControl): minimum zoom: " << _zoomMin;
    }
    updateProjection();
}

void CameraControl::zoomOut(const Vector2 zoomDiff) {
    zoomAdd(zoomDiff);
    if (_zoom >= _zoomMax) {
        Debug{} << "[!] (CameraControl): maximum zoom: " << _zoomMax;
    }
    updateProjection();
}

void CameraControl::zoomTo(const Vector2 zoom) {
    setZoom(zoom);
}

void CameraControl::zoomToMax(const Vector2 zoomDiff) {
    while (_zoom.x() < _zoomMax.x() || _zoom.y() < _zoomMax.y()) {
        auto newZoom = Vector2(std::min(_zoom.x() + zoomDiff.x(), _zoomMax.x()),
                        std::min(_zoom.y() + zoomDiff.y(), _zoomMax.y()));
        setZoom(newZoom);
    }
}

void CameraControl::zoomToMin(const Vector2 zoomDiff) {
    while (_zoom.x() > _zoomMin.x() || _zoom.y() > _zoomMin.y()) {
        auto newZoom = Vector2(std::max(_zoom.x() - zoomDiff.x(), _zoomMin.x()),
                        std::max(_zoom.y() - zoomDiff.y(), _zoomMin.y()));
        setZoom(newZoom);
    }
}

void CameraControl::zoomToAnchor(const Vector2 zoomDiff) {
    if (_zoom == _zoomAnchor) {
        return;
    }

    Vector2 newZoom;

    if ((_zoom.x() > _zoomAnchor.x() && _zoom.y() > _zoomAnchor.y())) {
        newZoom = zoomSub(zoomDiff);

        if ((zoomDiff.x() > 0 && newZoom.x() < _zoomAnchor.x()) ||
            (zoomDiff.y() > 0 && newZoom.y() < _zoomAnchor.y())) {
            newZoom = _zoomAnchor;
        }
    } else {
        newZoom = zoomAdd(zoomDiff);

        if ((zoomDiff.x() > 0 && newZoom.x() > _zoomAnchor.x()) ||
            (zoomDiff.y() > 0 && newZoom.y() > _zoomAnchor.y())) {
            newZoom = _zoomAnchor;
        }
    }

    _zoom = newZoom;
    updateProjection();
}

void CameraControl::OnScrollEvent(Platform::Sdl2Application::ScrollEvent& event) {
    if(_disableInput) {
        return;
    }

    if(event.offset().y() > 0) {
        zoomIn(_zoomStep);
    }

    if(event.offset().y() < 0) {
        zoomOut(_zoomStep);
    }

    event.setAccepted(true);
}

void CameraControl::OnKeyPressEvent(Platform::Sdl2Application::KeyEvent &event) {
    if(_disableInput) {
        return;
    }

    if (event.isAccepted()) {
        Warning() << "[!~] Key event passed to CameraControl already accepted";
        return;
    }

    if(event.key() == Platform::Sdl2Application::Key::Equal) {
        zoomIn(_zoomStep*2);
        event.setAccepted(true);
    }

    if(event.key() == Platform::Sdl2Application::Key::Minus) {
        zoomOut(_zoomStep*2);
        event.setAccepted(true);
    }
}

void CameraControl::updateProjection() const {
    _camera->setProjectionMatrix(Matrix3::projection(_zoom));
}

void CameraControl::startZoomingByVelocity() {
    _disableInput = true;
}

void CameraControl::stopZoomingByVelocity() {
    _disableInput = false;
}

bool CameraControl::isZoomingByVelocity() const {
    return _disableInput;
}

void CameraControl::updateZoomByVelocity(const Vector2 velocity, const Float dt, const bool thrusting, const bool immediate) {
    const Float speed = velocity.length();
    const Vector2 desired = zoomForSpeed(speed);

    if(immediate) {
        _zoomStoppedTime = 0.0f;
        setZoom(desired);
        startZoomingByVelocity();
        return;
    }

    startZoomingByVelocity();

    if(desired.x() > _zoom.x()) {
        _zoomStoppedTime = 0.0f;
        setZoom(desired);
        return;
    }

    if(thrusting || speed >= ZoomFlySpeed) {
        _zoomStoppedTime = 0.0f;
        return;
    }

    _zoomStoppedTime += dt;
    if(_zoomStoppedTime < ZoomStopHold)
        return;

    const Float travel = ZoomReturnRate*Math::min(dt, 0.05f);
    const Vector2 delta = _zoomAnchor - _zoom;
    setZoom(_zoom + Vector2{
        Math::clamp(delta.x(), -travel, travel),
        Math::clamp(delta.y(), -travel, travel)
    });
}

bool CameraControl::isZoomCloseToAnchor(const float epsilon) const {
    return (getZoom() - getZoomAnchor()).length() <= epsilon;
}

void CameraControl::setZoom(const Vector2& zoom) {
    _zoom = Math::clamp(zoom, _zoomMin, _zoomMax);
    updateProjection();
}

void CameraControl::setZoomMax(const Vector2& zoomMax) {
    _zoomMax = zoomMax;
    if (_zoom > _zoomMax) {
        _zoom = _zoomMax;
        updateProjection();
    }
}

void CameraControl::setZoomMin(const Vector2& zoomMin) {
    _zoomMin = zoomMin;
    if (_zoom < _zoomMin) {
        _zoom = _zoomMin;
        updateProjection();
    }
}

void CameraControl::setZoomStep(const Vector2& zoomStep) {
    _zoomStep = zoomStep;
}

void CameraControl::setZoomAnchor(const Vector2& zoomAnchor) {
    _zoomAnchor = zoomAnchor;
}

Vector2 CameraControl::getZoom() const {
    return _zoom;
}

Vector2 CameraControl::getZoomMax() const {
    return _zoomMax;
}

Vector2 CameraControl::getZoomMin() const {
    return _zoomMin;
}

Vector2 CameraControl::getZoomStep() const {
    return _zoomStep;
}

Vector2 CameraControl::getZoomAnchor() const {
    return _zoomAnchor;
}

bool CameraControl::isInputDisabled() const {
    return _disableInput;
}

Vector2 CameraControl::zoomForSpeed(const Float speed) const {
    if(speed <= ZoomFlySpeed)
        return _zoomAnchor;

    const Float t = Math::clamp(
        (speed - ZoomFlySpeed)/(ZoomSpeedAtMax - ZoomFlySpeed),
        0.0f, 1.0f);
    const Vector2 far{
        Math::min(_zoomMax.x(), ZoomOutMax),
        Math::min(_zoomMax.y(), ZoomOutMax)};
    return Math::lerp(_zoomAnchor, far, t);
}

} // namespace MoonLander
