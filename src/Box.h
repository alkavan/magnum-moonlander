// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_BOX_H
#define MOONLANDER_BOX_H

#include "MoonLander.h"
#include "DrawableMesh.h"

namespace MoonLander {
    class Box {
    private:
        Object2D &_object;
        b2BodyId _bodyId;
        DrawableMesh &_drawable;
    public:
        Box(Object2D &object, const b2BodyId bodyId, DrawableMesh &drawable);
        void update(const Float dt) const;
    };
}

#endif //MOONLANDER_BOX_H
