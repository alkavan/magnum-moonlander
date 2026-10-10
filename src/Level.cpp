// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "Level.h"
#include "TerrainGenerator.h"

#include <Magnum/Math/Functions.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Shaders/Flat.h>
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Trade/MeshData.h>

#include <random>
#include <vector>
#include <chrono>

namespace MoonLander {

Level::Level(Scene2D &scene, World &world) : _scene(scene), _world(world) {
    _boxMesh = MeshTools::compile(Primitives::squareSolid());
    _circleMesh = MeshTools::compile(Primitives::circle2DSolid(32));
}

Box *Level::newBox(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                   const Vector2 size, const Color4 color, const Float density) {
    const auto object = new Object2D{&_scene};
    object->setScaling(size);
    const auto bodyId = _world.createBox(object, transformation, size, b2_dynamicBody, density);
    const auto drawable = new DrawableMesh{*object, _boxMesh, _shader, color, drawable_group};

    return new Box{*object, bodyId, *drawable};
}

Box *Level::newBoxStatic(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                         const Vector2 size, const Color4 color) {
    const auto object = new Object2D{&_scene};
    object->setScaling(size);
    const auto bodyId = _world.createBox(object, transformation, size, b2_staticBody, 1.0f);
    const auto drawable = new DrawableMesh{*object, _boxMesh, _shader, color, drawable_group};

    return new Box{*object, bodyId, *drawable};
}

Circle *Level::newCircle(SceneGraph::DrawableGroup2D &drawable_group, const DualComplex transformation,
                         const Float radius, const Color4 color, const Float density) {
    const auto object = new Object2D{&_scene};
    object->setScaling({radius, radius});
    const auto bodyId = _world.createCircle(object, transformation, radius, b2_dynamicBody, density);
    const auto drawable = new DrawableMesh{*object, _circleMesh, _shader, color, drawable_group};

    return new Circle{*object, bodyId, *drawable};
}

void Level::buildMoonLandscape() {
    // Several camera-widths across (camera projection is ~50). Segment size
    // keeps polygon count reasonable while still looking continuous.
    constexpr Float width = 900.0f;
    constexpr Float segmentSize = 2.0f;
    const UnsignedInt seed = static_cast<UnsignedInt>(
        std::chrono::steady_clock::now().time_since_epoch().count());

    const TerrainProfile profile = TerrainGenerator::generate(width, segmentSize, seed);
    const auto& surface = profile.surface;
    const Float baseY = profile.baseY;
    const int segments = static_cast<int>(surface.size()) - 1;
    if (segments < 1) return;

    // Physics: one static convex quad per segment
    for (int i = 0; i < segments; ++i) {
        const Vector2 a = surface[i].position;
        const Vector2 b = surface[i + 1].position;
        if (Math::abs(b.x() - a.x()) < 0.01f) continue;

        Vector2 pts[4] = {
            {a.x(), a.y()},
            {b.x(), b.y()},
            {b.x(), baseY},
            {a.x(), baseY}
        };
        _world.createPolygon(
            nullptr,
            DualComplex::translation(Vector2{}),
            Containers::arrayView(pts),
            b2_staticBody,
            0.0f,
            0.9f);
    }

    // Visual mesh for the full terrain
    Containers::Array<Vector2> verts{NoInit, static_cast<std::size_t>(segments) * 6};
    std::size_t v = 0;
    for (int i = 0; i < segments; ++i) {
        const Vector2 a = surface[i].position;
        const Vector2 b = surface[i + 1].position;
        verts[v++] = {a.x(), a.y()};
        verts[v++] = {b.x(), b.y()};
        verts[v++] = {b.x(), baseY};
        verts[v++] = {a.x(), a.y()};
        verts[v++] = {b.x(), baseY};
        verts[v++] = {a.x(), baseY};
    }

    GL::Buffer terrainBuffer;
    terrainBuffer.setData(verts);
    _terrainMesh = GL::Mesh{};
    _terrainMesh.setCount(static_cast<Int>(verts.size()))
        .setPrimitive(GL::MeshPrimitive::Triangles)
        .addVertexBuffer(std::move(terrainBuffer), 0, Shaders::FlatGL2D::Position{});

    _terrainObject = new Object2D{&_scene};
    _terrainDrawable = new DrawableMesh{
        *_terrainObject, _terrainMesh, _shader,
        0x8a9bb0_rgbf, _groundGroup};

    // Landing pad markers: runs of consecutive pad points
    std::vector<std::pair<int, int>> pads;
    int runStart = -1;
    for (int i = 0; i < static_cast<int>(surface.size()); ++i) {
        if (surface[i].isPad) {
            if (runStart < 0) runStart = i;
        } else if (runStart >= 0) {
            pads.emplace_back(runStart, i - 1);
            runStart = -1;
        }
    }
    if (runStart >= 0) pads.emplace_back(runStart, static_cast<int>(surface.size()) - 1);

    if (!pads.empty()) {
        Containers::Array<Vector2> padVerts{NoInit, pads.size() * 6};
        std::size_t pv = 0;
        for (const auto& [aIdx, bIdx] : pads) {
            const Vector2 a = surface[aIdx].position;
            const Vector2 b = surface[bIdx].position;
            const Float top = a.y() + 0.15f;
            const Float bot = a.y() - 0.05f;
            padVerts[pv++] = {a.x(), top};
            padVerts[pv++] = {b.x(), top};
            padVerts[pv++] = {b.x(), bot};
            padVerts[pv++] = {a.x(), top};
            padVerts[pv++] = {b.x(), bot};
            padVerts[pv++] = {a.x(), bot};
        }
        GL::Buffer padBuffer;
        padBuffer.setData(padVerts);
        _padMesh = GL::Mesh{};
        _padMesh.setCount(static_cast<Int>(padVerts.size()))
            .setPrimitive(GL::MeshPrimitive::Triangles)
            .addVertexBuffer(std::move(padBuffer), 0, Shaders::FlatGL2D::Position{});
        _padObject = new Object2D{&_scene};
        _padDrawable = new DrawableMesh{
            *_padObject, _padMesh, _shader,
            0x66dd88_rgbf, _groundGroup};
    }
}

void Level::initialize() {
    buildMoonLandscape();
}

void Level::update(const Float dt) {
    for (const auto &box : _boxes) {
        box->update(dt);
    }

    for (const auto &circle : _circles) {
        circle->update(dt);
    }

    for (const auto &particle : _particles) {
        particle->update(dt);
    }
}

void Level::draw(SceneGraph::Camera2D &camera) {
    camera.draw(_groundGroup);
    camera.draw(_boxGroup);
    camera.draw(_circleGroup);
    camera.draw(_particleGroup);
}

void Level::addBox(const DualComplex &transformation) {
    const auto box = newBox(
        _boxGroup,
        transformation,
        {0.5f, 0.5f},
        0xffff66_rgbf,
        1.0f);

    arrayAppend(_boxes, box);
}

void Level::addCircle(const DualComplex &transformation, const Float radius) {
    const auto circle = newCircle(
        _circleGroup,
        transformation,
        radius,
        0x66ffcc_rgbf,
        1.0f);

    arrayAppend(_circles, circle);
}

void Level::addParticleCloud(const Vector2 &center, const int count, const Float spread) {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<Float> dist(-spread, spread);

    for (int i = 0; i < count; ++i) {
        const Vector2 offset{dist(rng), dist(rng)};
        if (offset.dot() > spread * spread) continue;

        const Vector2 pos = center + offset;
        const DualComplex transform = DualComplex::translation(pos);

        const auto particle = newCircle(
            _particleGroup,
            transform,
            0.06f,
            0xffaa44_rgbf,
            0.3f);

        arrayAppend(_particles, particle);
    }
}

} // namespace MoonLander
