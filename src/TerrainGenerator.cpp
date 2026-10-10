// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "TerrainGenerator.h"

#include <Magnum/Math/Functions.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace MoonLander {

Float TerrainGenerator::hashToUnit(UnsignedInt n) {
    // Integer hash -> [0, 1)
    n = (n << 13u) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return static_cast<Float>(n & 0x7fffffffu) / static_cast<Float>(0x7fffffffu);
}

Float TerrainGenerator::valueNoise(const Float x, const UnsignedInt seed) {
    const auto i0 = static_cast<int>(Math::floor(x));
    const Float f = x - static_cast<Float>(i0);
    // Smoothstep
    const Float u = f * f * (3.0f - 2.0f * f);

    const Float a = hashToUnit(static_cast<UnsignedInt>(i0) * 374761393u + seed);
    const Float b = hashToUnit(static_cast<UnsignedInt>(i0 + 1) * 374761393u + seed);
    return Math::lerp(a, b, u) * 2.0f - 1.0f; // [-1, 1]
}

Float TerrainGenerator::fbm(const Float x, const UnsignedInt seed, const int octaves) {
    Float sum = 0.0f;
    Float amp = 1.0f;
    Float freq = 1.0f;
    Float norm = 0.0f;
    for (int o = 0; o < octaves; ++o) {
        sum += amp * valueNoise(x * freq, seed + static_cast<UnsignedInt>(o) * 1013u);
        norm += amp;
        amp *= 0.5f;
        freq *= 2.1f;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

TerrainProfile TerrainGenerator::generate(const Float width, const Float segmentSize, const UnsignedInt seed) {
    TerrainProfile profile;
    if (width <= 0.0f || segmentSize <= 0.0f) return profile;

    const int segments = std::max(8, static_cast<int>(width / segmentSize));
    const Float step = width / static_cast<Float>(segments);
    const Float xMin = -width * 0.5f;

    std::vector<Vector2> surface(segments + 1);
    Float minY = 1e9f;
    Float maxY = -1e9f;

    // Base rolling terrain
    for (int i = 0; i <= segments; ++i) {
        const Float x = xMin + i * step;
        const Float nx = x * 0.012f; // spatial frequency
        Float y = -8.0f;
        y += fbm(nx, seed, 4) * 6.5f;          // large ridges / basins
        y += fbm(nx * 3.7f, seed + 17u, 3) * 2.2f; // medium jaggedness
        y += fbm(nx * 11.0f, seed + 91u, 2) * 0.7f; // fine grit
        surface[i] = {x, y};
        minY = Math::min(minY, y);
        maxY = Math::max(maxY, y);
    }

    std::mt19937 rng{seed};
    std::uniform_real_distribution<Float> unit(0.0f, 1.0f);

    // Craters: smooth bowl dips
    const int craterCount = 4 + static_cast<int>(unit(rng) * 5.0f);
    for (int c = 0; c < craterCount; ++c) {
        const Float cx = xMin + unit(rng) * width;
        const Float radius = 6.0f + unit(rng) * 14.0f;
        const Float depth = 2.5f + unit(rng) * 4.5f;
        for (auto& p : surface) {
            if (const Float d = Math::abs(p.x() - cx); d < radius) {
                const Float t = d / radius;
                const Float bowl = (1.0f - t * t) * depth;
                p.y() -= bowl;
            }
        }
    }

    // Occasional cliffs / ridges
    const int cliffCount = 3 + static_cast<int>(unit(rng) * 4.0f);
    for (int c = 0; c < cliffCount; ++c) {
        const Float cx = xMin + unit(rng) * width;
        const Float rise = (unit(rng) < 0.5f ? -1.0f : 1.0f) * (3.0f + unit(rng) * 5.0f);
        const Float widthC = 2.0f + unit(rng) * 4.0f;
        for (auto& p : surface) {
            if (const Float d = p.x() - cx; Math::abs(d) < widthC) {
                const Float t = d / widthC; // -1..1
                p.y() += rise * (0.5f + 0.5f * t);
            }
        }
    }

    // Keep the playable band below the lander's start altitude (~10)
    // and recompute bounds after features.
    minY = 1e9f;
    maxY = -1e9f;
    for (auto& p : surface) {
        p.y() = Math::clamp(p.y(), -28.0f, 2.0f);
        minY = Math::min(minY, p.y());
        maxY = Math::max(maxY, p.y());
    }

    // Landing pads: flatten a run of points at the local height
    const int padCount = 5 + static_cast<int>(unit(rng) * 4.0f);
    std::vector<bool> isPadPoint(surface.size(), false);
    for (int p = 0; p < padCount; ++p) {
        const int padSegments = 2 + static_cast<int>(unit(rng) * 4.0f); // 2..5 segments
        const int start = 2 + static_cast<int>(unit(rng) * static_cast<Float>(segments - padSegments - 4));
        if (start < 1 || start + padSegments >= segments) continue;
        const Float padY = surface[start].y();
        for (int i = start; i <= start + padSegments; ++i) {
            surface[i].y() = padY;
            isPadPoint[i] = true;
        }
    }

    profile.baseY = minY - 12.0f;
    profile.width = width;
    profile.surface = Containers::Array<TerrainPoint>{NoInit, surface.size()};
    for (std::size_t i = 0; i < surface.size(); ++i) {
        profile.surface[i] = TerrainPoint{surface[i], isPadPoint[i]};
    }
    return profile;
}

} // namespace MoonLander
