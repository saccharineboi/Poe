// Poe: OpenGL 4.5 Renderer
// Copyright (C) 2024 Omar Huseynov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Constants.hpp"
#include "Suppress.hpp"

SUPPRESS_WARNINGS()
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
ENABLE_WARNINGS()

#include <cmath>
#include <vector>
#include <cassert>
#include <limits>
#include <string>

namespace Poe::Utility
{
    ////////////////////////////////////////
    struct PairHash
    {
        template <typename T1, typename T2>
        size_t operator()(const std::pair<T1, T2>& p) const
        {
            auto hash1 = std::hash<T1>()(p.first);
            auto hash2 = std::hash<T2>()(p.second);
            return std::hash<size_t>()(hash1 ^ hash2);
        }
    };

    ////////////////////////////////////////
    float ComputeDeltaTime();

    ////////////////////////////////////////
    template <typename T, typename TS>
    T Lerp(const T& x0, const T& x1, const TS s)
    { return x0 + (x1 - x0) * s; }

    ////////////////////////////////////////
    inline bool FloatEquals(float a, float b, float epsilon = EPSILON)
    {
        return std::abs(a - b) <= epsilon;
    }

    ////////////////////////////////////////
    glm::vec3 ComputeFrustumCenter(const std::vector<glm::vec4>& frustumCorners);

    ////////////////////////////////////////
    glm::mat4 FitLightProjectionToFrustum(const glm::mat4& lightView, const std::vector<glm::vec4>& frustumCorners, float zMult);
}
