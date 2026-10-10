// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_CAMERACONTROL_H
#define MOONLANDER_CAMERACONTROL_H

#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Math/Functions.h>

#include "MoonLander.h"

namespace MoonLander {
    using namespace Math::Literals;

    class CameraControl {
    public:
        explicit CameraControl(Object2D *cameraObject);

        [[nodiscard]] SceneGraph::Camera2D &getCamera() const;
        [[nodiscard]] const Object2D &getCameraObject() const;

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
        void OnScrollEvent(Platform::Sdl2Application::ScrollEvent &event);
        void OnKeyPressEvent(Platform::Sdl2Application::KeyEvent &event);

        void updateProjection() const;

        /// Mark that velocity is adjusting the zoom
        void startZoomingByVelocity();
        /// Mark that zooming by velocity has stopped
        void stopZoomingByVelocity();
        /// Check if the camera is currently zooming due to velocity
        [[nodiscard]] bool isZoomingByVelocity() const;

        /**
         * Zoom out linearly with flight speed. A reversal holds the lens,
         * and a real stop walks back to the anchor at a fixed rate.
         */
        void updateZoomByVelocity(Vector2 velocity, Float dt, bool thrusting, bool immediate = false);

        /**
         * @brief Checks if the current zoom level is close enough to the zoom anchor.
         * @param epsilon Precision threshold to determine closeness.
         * @return True if zoom is close to the anchor within the given epsilon, false otherwise.
         */
        [[nodiscard]] bool isZoomCloseToAnchor(float epsilon) const;

        // Setter methods
        void setZoom(const Vector2& zoom);
        void setZoomMax(const Vector2& zoomMax);
        void setZoomMin(const Vector2& zoomMin);
        void setZoomStep(const Vector2& zoomStep);
        void setZoomAnchor(const Vector2& zoomAnchor);

        // Getter methods
        [[nodiscard]] Vector2 getZoom() const;
        [[nodiscard]] Vector2 getZoomMax() const;
        [[nodiscard]] Vector2 getZoomMin() const;
        [[nodiscard]] Vector2 getZoomStep() const;
        [[nodiscard]] Vector2 getZoomAnchor() const;
        [[nodiscard]] bool isInputDisabled() const;

    private:
        SceneGraph::Camera2D *_camera;
        Object2D *_cameraObject;

        Vector2 _zoom {1.0f, 1.0f};
        Vector2 _zoomMax = {100.0f, 100.0f};
        Vector2 _zoomMin = {0.0f, 0.0f};
        Vector2 _zoomStep = {5.0f, 5.0f};
        Vector2 _zoomAnchor = {50.0f, 50.0f};

        // Flag to indicate if zooming is driven by velocity
        bool _disableInput = false;
        Float _zoomStoppedTime = 0.0f;

        // Projection size grows linearly with flight speed until a capped zoom-out.
        // Below ZoomFlySpeed the body is settling, not flying.
        static constexpr Float ZoomFlySpeed = 1.5f;
        static constexpr Float ZoomSpeedAtMax = 8.0f;
        // Closer than the absolute max (100). Anchor is 50.
        static constexpr Float ZoomOutMax = 70.0f;
        // Longer than a thrust reversal through zero, so that dip is not a landing.
        static constexpr Float ZoomStopHold = 0.5f;
        // Projection units per second. Far zoom (70) back to the anchor (50) takes 2.5s.
        static constexpr Float ZoomReturnRate = 8.0f;

        [[nodiscard]] Vector2 zoomForSpeed(Float speed) const;
    };
}

#endif //MOONLANDER_CAMERACONTROL_H
