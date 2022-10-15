// Poe: OpenGL Renderer
// Copyright (C) 2022 saccharineboi
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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>

namespace Poe
{
    ////////////////////////////////////////
    struct FirstPersonCamera;
    struct FogUB;

    ////////////////////////////////////////
    struct DebugUI
    {
        DebugUI() = delete;

        static void Init(GLFWwindow* window);
        static void Destroy();
        static void NewFrame();
        static void EndFrame();

        static void Begin_GlobalInfo();
        static void End_GlobalInfo();

        static bool mEnableWireframe;
        static bool mEnableSkybox;
        static bool mEnableGrid;
        static void Draw_GlobalInfo_General();

        static void Draw_GlobalInfo_Camera(FirstPersonCamera&);

        static float mGamma;
        static float mExposure;
        static float mGrayscaleWeight;
        static float mKernelWeight;
        static void Draw_GlobalInfo_PostProcess();

        static void Draw_GlobalInfo_Fog(FogUB&);

        static inline constexpr int MAX_COUT_LOGS = 500;
        static inline constexpr int MAX_CERR_LOGS = 500;

        static std::vector<std::string> mCoutLogs;
        static std::vector<std::string> mCerrLogs;

        static void PushLog(FILE* file, const char* format, ...);

        static void Render_LogInfo();
    };
}
