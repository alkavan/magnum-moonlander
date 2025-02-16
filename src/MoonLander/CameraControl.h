#ifndef MAGNUM_MOONLANDER_CAMERACONTROL_H
#define MAGNUM_MOONLANDER_CAMERACONTROL_H

#include <Magnum/SceneGraph/Camera.h>

#include "Game.h"

namespace Magnum::Game {
    using namespace Math::Literals;
    using Platform::Sdl2Application;

    class CameraControl {
    public:
        explicit CameraControl(Object2D *cameraObject);

        [[nodiscard]] SceneGraph::Camera2D &getCamera() const {
            return *_camera;
        };

        [[nodiscard]] const Object2D &getCameraObject() const {
            return *_cameraObject;
        };

        [[nodiscard]] Vector2 getContainerTranslation() const;
        [[nodiscard]] Vector2 projectedPosition(Vector2 screenPosition, Vector2 screenSize) const;

        void move(Vector2 displacement) const;
        void moveTo(Vector2 point) const;

        Vector2 zoomAdd(Vector2 zoomDiff);
        Vector2 zoomSub(Vector2 zoomDiff);

        void zoomIn(Vector2 zoomDiff);
        void zoomOut(Vector2 zoomDiff);

        void zoomTo(Vector2 zoom);
        void zoomToMax(Vector2 zoomDiff);
        void zoomToMin(Vector2 zoomDiff);
        void zoomToAnchor(Vector2 zoomDiff);

        // events
        void OnScrollEvent(Sdl2Application::ScrollEvent &event);
        void OnKeyPressEvent(Sdl2Application::KeyEvent &event);

        void updateProjection() const;

        /// Mark that the zoom is being adjusted by velocity
        void startZoomingByVelocity();

        /// Mark that zooming by velocity has stopped
        void stopZoomingByVelocity();

        /// Check if the camera is currently zooming due to velocity
        [[nodiscard]] bool isZoomingByVelocity() const;

        /**
         * @brief Checks if the current zoom level is sufficiently close to the zoom anchor.
         * @param epsilon Precision threshold to determine closeness.
         * @return True if zoom is close to the anchor within the given epsilon, false otherwise.
         */
        [[nodiscard]] bool isZoomCloseToAnchor(float epsilon) const;

    private:
        SceneGraph::Camera2D *_camera;
        Object2D *_cameraObject;

        Vector2 _zoom;
        Vector2 _zoomMax = {100.0f, 100.0f};
        Vector2 _zoomMin = {0.0f, 0.0f};
        Vector2 _zoomStep = {5.0f, 5.0f};
        Vector2 _zoomAnchor = {50.0f, 50.0f};

        // Flag to indicate if zooming is driven by velocity
        bool _disableInput = false;

    public:
        // Setter methods
        void setZoom(const Vector2& zoom)
        {
            _zoom = Math::clamp(zoom, _zoomMin, _zoomMax);
            updateProjection();
        }

        void setZoomMax(const Vector2& zoomMax)
        {
            _zoomMax = zoomMax;
            if (_zoom > _zoomMax)
            {
                _zoom = _zoomMax;
                updateProjection();
            }
        }

        void setZoomMin(const Vector2& zoomMin)
        {
            _zoomMin = zoomMin;
            if (_zoom < _zoomMin)
            {
                _zoom = _zoomMin;
                updateProjection();
            }
        }

        void setZoomStep(const Vector2& zoomStep)
        {
            _zoomStep = zoomStep;
        }

        void setZoomAnchor(const Vector2& zoomAnchor)
        {
            _zoomAnchor = zoomAnchor;
        }

        // Getter methods
        [[nodiscard]] Vector2 getZoom() const {
            return _zoom;
        }

        [[nodiscard]] Vector2 getZoomMax() const
        {
            return _zoomMax;
        }

        [[nodiscard]] Vector2 getZoomMin() const
        {
            return _zoomMin;
        }

        [[nodiscard]] Vector2 getZoomStep() const
        {
            return _zoomStep;
        }

        [[nodiscard]] Vector2 getZoomAnchor() const
        {
            return _zoomAnchor;
        }

        [[nodiscard]] bool isInputDisabled() const
        {
            return _disableInput;
        }
    };

    /**
     * Constructor.
     * @param cameraObject
     */
    inline CameraControl::CameraControl(Object2D *cameraObject): _cameraObject(cameraObject), _zoom(50.0f) {
        const auto camera = new SceneGraph::Camera2D{*_cameraObject};

        camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
                .setViewport(GL::defaultFramebuffer.viewport().size());

        camera->setProjectionMatrix(Matrix3::projection(_zoom));

        _camera = camera;
    }

    inline Vector2 CameraControl::zoomAdd(const Vector2 zoomDiff) {
        _zoom += zoomDiff;
        if(_zoom > _zoomMax) {
            _zoom = _zoomMax;
        }
        return _zoom;
    }

    inline Vector2 CameraControl::zoomSub(const Vector2 zoomDiff) {
        _zoom -= zoomDiff;
        if(_zoom < _zoomMin) {
            _zoom = _zoomMin;
        }
        return _zoom;
    }

    inline void CameraControl::zoomIn(const Vector2 zoomDiff) {
        zoomSub(zoomDiff);
        if (_zoom <= _zoomMin) {
            Debug{} << "[!] (CameraControl): minimum zoom: " << _zoomMin;
        }
        updateProjection();
    }

    inline void CameraControl::zoomOut(const Vector2 zoomDiff) {
        zoomAdd(zoomDiff);
        if (_zoom >= _zoomMax) {
            Debug{} << "[!] (CameraControl): maximum zoom: " << _zoomMax;
        }
        updateProjection();
    }

    // Method to zoom to the maximum zoom level
    inline void CameraControl::zoomToMax(const Vector2 zoomDiff) {
        // Increment zoom until it reaches _zoomMax
        while (_zoom.x() < _zoomMax.x() || _zoom.y() < _zoomMax.y()) {
            auto newZoom = Vector2(std::min(_zoom.x() + zoomDiff.x(), _zoomMax.x()),
                            std::min(_zoom.y() + zoomDiff.y(), _zoomMax.y()));
            setZoom(newZoom);
        }
    }

    // Method to zoom to the minimum zoom level
    inline void CameraControl::zoomToMin(const Vector2 zoomDiff) {
        // Decrement zoom until it reaches _zoomMin
        while (_zoom.x() > _zoomMin.x() || _zoom.y() > _zoomMin.y()) {
            auto newZoom = Vector2(std::max(_zoom.x() - zoomDiff.x(), _zoomMin.x()),
                            std::max(_zoom.y() - zoomDiff.y(), _zoomMin.y()));
            setZoom(newZoom);
        }
    }

    /// Zoom the camera towards or away from the predefined anchor position (_zoomAnchor).
    /// It adjusts the zoom depending on whether the current zoom is greater or less than the anchor.
    ///
    /// @param zoomDiff The incremental difference by which to zoom in or out.
    inline void CameraControl::zoomToAnchor(const Vector2 zoomDiff) {
        // If already at _zoomAnchor, stop zooming
        if (_zoom == _zoomAnchor) {
            return;
        }

        Vector2 newZoom;

        // Determine direction: zoom in if _zoom > _zoomAnchor, zoom out otherwise
        if ((_zoom.x() > _zoomAnchor.x() && _zoom.y() > _zoomAnchor.y())) {
            // Zoom in (reduce the zoom values)
            newZoom = zoomSub(zoomDiff);

            // Clamp to _zoomAnchor when overshooting past the anchor inwards
            if ((zoomDiff.x() > 0 && newZoom.x() < _zoomAnchor.x()) ||
                (zoomDiff.y() > 0 && newZoom.y() < _zoomAnchor.y())) {
                newZoom = _zoomAnchor;
                }
        } else {
            // Zoom out (increase the zoom values)
            newZoom = zoomAdd(zoomDiff);

            // Clamp to _zoomAnchor when overshooting past the anchor outwards
            if ((zoomDiff.x() > 0 && newZoom.x() > _zoomAnchor.x()) ||
                (zoomDiff.y() > 0 && newZoom.y() > _zoomAnchor.y())) {
                newZoom = _zoomAnchor;
                }
        }

        _zoom = newZoom; // Update the zoom
        updateProjection(); // Apply the updated zoom to the camera projection
    }

    /// Mark that the zoom is being adjusted by the velocity
    inline void CameraControl::startZoomingByVelocity() {
        _disableInput = true;
    }

    /// Mark that zooming by velocity has stopped
    inline void CameraControl::stopZoomingByVelocity() {
        _disableInput = false;
    }

    /// Check if the camera is currently zooming due to velocity
    inline bool CameraControl::isZoomingByVelocity() const {
        return _disableInput;
    }

    inline void CameraControl::move(const Vector2 displacement) const
    {
        _cameraObject->translate(displacement * _zoom);
    }

    inline void CameraControl::moveTo(const Vector2 point) const
    {
        const auto positionDiff = point - _cameraObject->translation();
        _cameraObject->translate(positionDiff);
    }

    inline Vector2 CameraControl::projectedPosition(const Vector2 screenPosition, const Vector2 screenSize) const {
        return _camera->projectionSize() * Vector2::yScale(-1.0f) *
                              (screenPosition / screenSize - Vector2{0.5f});
    }

    inline Vector2 CameraControl::getContainerTranslation() const {
        return _cameraObject->translation();
    }

    inline void CameraControl::OnScrollEvent(Sdl2Application::ScrollEvent& event)
    {
        if(_disableInput) {
            return;
        }

        // zoom-in
        if(event.offset().y() > 0) {
            zoomIn(_zoomStep);
        }

        // zoom-out
        if(event.offset().y() < 0) {
            zoomOut(_zoomStep);
        }

        event.setAccepted(true);
    }

    inline void CameraControl::OnKeyPressEvent(Sdl2Application::KeyEvent &event)
    {
        if(_disableInput) {
            return;
        }

        if (event.isAccepted()) {
            Warning() << "[!~] Key event passed to CameraControl already accepted";
            return;
        }

        if(event.key() == Sdl2Application::Key::Equal) {
            zoomIn(_zoomStep*2);
            event.setAccepted(true);
        }

        if(event.key() == Sdl2Application::Key::Minus) {
            zoomOut(_zoomStep*2);
            event.setAccepted(true);
        }
    }

    inline void CameraControl::updateProjection() const
    {
        _camera->setProjectionMatrix(Matrix3::projection(_zoom));
    }

    inline bool CameraControl::isZoomCloseToAnchor(const float epsilon) const {
        return (getZoom() - getZoomAnchor()).length() <= epsilon;
    }

}
#endif //MAGNUM_MOONLANDER_CAMERACONTROL_H
