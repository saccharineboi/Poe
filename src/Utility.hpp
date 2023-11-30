// Poe: OpenGL Renderer
// Copyright (C) 2023 saccharineboi
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

#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cassert>
#include <limits>

namespace Poe::Utility
{
    ////////////////////////////////////////
    inline float ComputeDeltaTime()
    {
        static float lastTime;
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        return deltaTime;
    }

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
    inline glm::vec3 ComputeFrustumCenter(const std::vector<glm::vec4>& frustumCorners)
    {
        assert(8 == frustumCorners.size());

        glm::vec3 center(0.0f);
        for (const glm::vec4& corner : frustumCorners) {
            center += glm::vec3(corner);
        }
        return center / static_cast<float>(frustumCorners.size());
    }

    ////////////////////////////////////////
    inline glm::mat4 FitLightProjectionToFrustum(const glm::mat4& lightView,
                                                 const std::vector<glm::vec4>& frustumCorners,
                                                 float zMult)
    {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (const glm::vec4& corner : frustumCorners) {
            glm::vec4 p{ lightView * corner };

            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
            minZ = std::min(minZ, p.z);
            maxZ = std::max(maxZ, p.z);
        }

        minZ = minZ < 0.0f ? minZ * zMult : minZ / zMult;
        maxZ = maxZ < 0.0f ? maxZ / zMult : maxZ * zMult;

        return glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    }
}
