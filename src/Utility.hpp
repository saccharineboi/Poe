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

#include <GLFW/glfw3.h>
#include <cmath>

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
}
