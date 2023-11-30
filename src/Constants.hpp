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

namespace Poe
{
    ////////////////////////////////////////
    inline constexpr int POE_VERSION_MAJOR = 0;
    inline constexpr int POE_VERSION_MINOR = 1;

    ////////////////////////////////////////
    inline constexpr float PI = 3.1415926f;
    inline constexpr float PI2 = 6.2831853f;
    inline constexpr float PIH = 1.5707963f;
    inline constexpr float R2D = 57.295779f;
    inline constexpr float D2R = 0.017453293f;
    inline constexpr float EPSILON = 0.000001f;

    ////////////////////////////////////////
    inline constexpr int DIR_LIGHT_DEPTH_MAP_BIND_POINT{ 13 };
    inline constexpr int POINT_LIGHT_DEPTH_MAP_BIND_POINT{ 14 };
    inline constexpr int SPOT_LIGHT_DEPTH_MAP_BIND_POINT{ 15 };

    ////////////////////////////////////////
    inline constexpr int NUM_SHADOW_CASCADES{ 4 };
    inline constexpr int SHADOW_SIZE{ 2048 };
}
