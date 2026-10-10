// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_TERRAIN_GENERATOR_H
#define MOONLANDER_TERRAIN_GENERATOR_H

#include <Corrade/Containers/Array.h>
#include <Magnum/Magnum.h>

#include "MoonLander.h"

namespace MoonLander {

    /**
     * One surface sample. Consecutive samples form a segment.
     * isPad marks the start of a flat landing pad segment
     * (the following point shares the same height).
     */
    struct TerrainPoint {
        Vector2 position;
        bool isPad = false;
    };

    /**
     * Result of procedural generation.
     * surface[0] .. surface[n-1] is the height profile from left to right.
     * baseY is a solid floor well below the lowest sample.
     */
    struct TerrainProfile {
        Containers::Array<TerrainPoint> surface;
        Float baseY = -40.0f;
        Float width = 0.0f;
    };

    /**
     * Simple procedural lunar terrain.
     *
     * Layers a few octaves of value noise, then stamps craters, ridges,
     * and flat landing pads of varying width and elevation. Deterministic
     * for a given seed.
     *
     * Typical call for a level several camera-widths across:
     *   generate(800.0f, 2.0f, seed);
     */
    class TerrainGenerator {
    public:
        static TerrainProfile generate(Float width, Float segmentSize, UnsignedInt seed) ;

    private:
        static Float hashToUnit(UnsignedInt n);
        static Float valueNoise(Float x, UnsignedInt seed);
        static Float fbm(Float x, UnsignedInt seed, int octaves);
    };
}

#endif // MOONLANDER_TERRAIN_GENERATOR_H
