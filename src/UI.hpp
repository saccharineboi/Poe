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

#include "Cameras.hpp"
#include "Poe.hpp"
#include "Suppress.hpp"
#include "Utility.hpp"

SUPPRESS_WARNINGS()
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
ENABLE_WARNINGS()

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>

namespace Poe
{
    ////////////////////////////////////////
    struct FirstPersonCamera;
    struct PostProcessProgram;
    struct FogUB;
    struct EmissiveColorMaterial;
    struct DirLight;
    struct PbrLightMaterial;

    ////////////////////////////////////////
    struct DebugUI
    {
        DebugUI() = delete;

        static void Init(GLFWwindow* window)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 460 core");

            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontFromFileTTF("../fonts/VT323-Regular.ttf", 18);
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        static void Destroy()
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        static void NewFrame()
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        static void EndFrame()
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        static void Begin_GlobalInfo()
        {
            ImGui::SetNextWindowSize({ 400, 0 });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Poe Global Info (OpenGL 4.6 Core)");
        }

        static void End_GlobalInfo()
        {
            ImGui::End();
        }

        static bool mEnableWireframe;
        static bool mEnableSkybox;
        static bool mEnableGrid;
        static bool mEnableVsync;

        static void Draw_GlobalInfo_General()
        {
            ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[General]");
            ImGui::Text("%.2f MS, %.2f FPS", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Wireframe Mode", &mEnableWireframe);
            ImGui::Checkbox("Enable Skybox", &mEnableSkybox);
            ImGui::Checkbox("Enable Grid", &mEnableGrid);
            ImGui::Checkbox("Enable Vsync", &mEnableVsync);
            static float clearColor[]{ 0.01f, 0.01f, 0.01f };
            ImGui::ColorEdit3("Clear Color", clearColor);
            glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
            ImGui::NewLine();
        }

        static void Draw_GlobalInfo_Camera(FirstPersonCamera& camera)
        {
            ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Camera]");
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.mPosition.x, camera.mPosition.y, camera.mPosition.z);
            ImGui::Text("Direction: (%.2f, %.2f, %.2f)", camera.mDirection.x, camera.mDirection.y, camera.mDirection.z);
            ImGui::SliderFloat("Speed", &camera.mSpeed, 1.0f, 500.0f);
            float degrees = glm::degrees(camera.mFovy);
            ImGui::SliderFloat("FovY", &degrees, 1.0f, 180.0f);
            camera.mFovy = glm::radians(degrees);
            ImGui::SliderFloat("Near", &camera.mNear, 0.1f, 10.0f);
            ImGui::SliderFloat("Far", &camera.mFar, 10.0f, 1000.0f);
            ImGui::SliderFloat("Sensitivity", &camera.mSensitivity, 0.001f, 0.01f);
            ImGui::SliderFloat("Smoothness", &camera.mSmoothness, 1.0f, 20.0f);
            ImGui::NewLine();
        }

        static void Draw_GlobalInfo_PostProcess(PostProcessUB& block)
        {
            ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Post-Process]");

            float gamma{ block.GetGamma() };
            float exposure{ block.GetExposure() };
            float grayscaleWeight{ block.GetGrayscaleWeight() };
            float kernelWeight{ block.GetKernelWeight() };

            ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f);
            ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
            ImGui::SliderFloat("Grayscale", &grayscaleWeight, 0.0f, 1.0f);

            static int currentKernel{};
            static const char* kernelNames[]{ "Identity", "Sharpen", "Blur", "EdgeDetect", "Emboss" };
            if (ImGui::Combo("Kernel Type", &currentKernel, kernelNames, sizeof(kernelNames) / sizeof(char*)))
            {
                switch (currentKernel) {
                    case 0:
                        block.SetIdentityKernel();
                        break;
                    case 1:
                        block.SetSharpenKernel();
                        break;
                    case 2:
                        block.SetBlurKernel();
                        break;
                    case 3:
                        block.SetEdgeDetectKernel();
                        break;
                    case 4:
                        block.SetEmbossKernel();
                        break;
                }
            }

            ImGui::SliderFloat("Kernel", &kernelWeight, 0.0f, 1.0f);
            ImGui::NewLine();

            block.SetGamma(gamma);
            block.SetExposure(exposure);
            block.SetGrayscaleWeight(grayscaleWeight);
            block.SetKernelWeight(kernelWeight);
            block.Update();
        }

        static void Draw_GlobalInfo_Fog(FogUB& fogBlock)
        {
            ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Fog Settings]");
            float fogDistance = fogBlock.GetDistance();
            ImGui::SliderFloat("Distance", &fogDistance, 1.0f, 1000.0f);
            if (!Utility::FloatEquals(fogDistance, fogBlock.GetDistance()))
                fogBlock.SetDistance(fogDistance);
            float fogExponent = fogBlock.GetExponent();
            ImGui::SliderFloat("Exponent", &fogExponent, 0.01f, 3.0f);
            if (!Utility::FloatEquals(fogExponent, fogBlock.GetExponent()))
                fogBlock.SetExponent(fogExponent);
            glm::vec3 color = fogBlock.GetColor();
            float colorF[]{ color.x, color.y, color.z };
            ImGui::ColorEdit3("Color", colorF);
            color = glm::vec3(colorF[0], colorF[1], colorF[2]);
            if (color != fogBlock.GetColor())
                fogBlock.SetColor(color);
            ImGui::NewLine();
        }

        static inline constexpr int MAX_COUT_LOGS = 500;
        static inline constexpr int MAX_CERR_LOGS = 500;

        static inline constexpr float BG_ALPHA = 0.8f;

        static std::vector<std::string> mCoutLogs;
        static std::vector<std::string> mCerrLogs;

        static void PushLog(FILE* file, const char* format, ...)
        {
            if ((file == stdout && mCoutLogs.size() > MAX_COUT_LOGS) ||
                (file == stderr && mCerrLogs.size() > MAX_CERR_LOGS))
                return;

            std::va_list args;
            va_start(args, format);
#define BUFFER_MAX 1024
            char buffer[BUFFER_MAX];
            std::vsnprintf(buffer, BUFFER_MAX, format, args);
            va_end(args);
            if (file == stdout)
                mCoutLogs.push_back(buffer);
            else if (file == stderr)
                mCerrLogs.push_back(buffer);
        }

        static void Render_LogInfo()
        {
            ImGui::SetNextWindowSize({ 800, 0 });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);

            ImGui::Begin("stdout");
            ImGui::BeginChild("stdout logs", { -1, 600 });
            for (size_t i = 0; i < MAX_COUT_LOGS && i < mCoutLogs.size(); ++i)
                ImGui::TextWrapped("%s", mCoutLogs[i].c_str());
            ImGui::EndChild();
            ImGui::End();

            ImGui::SetNextWindowSize({ 800, 0 });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);

            ImGui::Begin("stderr");
            ImGui::BeginChild("stderr logs", { -1, 600 });
            for (size_t i = 0; i < MAX_CERR_LOGS && i < mCerrLogs.size(); ++i)
                ImGui::TextWrapped("%s", mCerrLogs[i].c_str());
            ImGui::EndChild();
            ImGui::End();
        }

        static void Render_EmissiveColorMaterialInfo(EmissiveColorMaterial& mat)
        {
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Emissive Color Material");
            float color4[] { mat.mColor.x, mat.mColor.y, mat.mColor.z, mat.mColor.w };
            ImGui::ColorEdit3("Color", color4);
            std::memcpy(glm::value_ptr(mat.mColor), color4, 16);
            ImGui::End();
        }

        static void Render_PbrLightMaterialInfo(PbrLightMaterial& material)
        {
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("PBR Light Material");

            float albedo[] { material.mAlbedo.x, material.mAlbedo.y, material.mAlbedo.z };
            ImGui::ColorEdit3("Albedo", albedo);
            std::memcpy(glm::value_ptr(material.mAlbedo), albedo, 12);

            ImGui::NewLine();

            ImGui::SliderFloat("Metallic", &material.mMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &material.mRoughness, 0.0f, 1.0f);
            ImGui::SliderFloat("AO", &material.mAo, 0.0f, 1.0f);

            ImGui::End();
        }

        static void Render_DirLightInfo(DirLight& dirLight)
        {
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Directional Light #0");

            float color3[] { dirLight.mColor.x, dirLight.mColor.y, dirLight.mColor.z };
            ImGui::ColorEdit3("Light Color", color3);
            std::memcpy(glm::value_ptr(dirLight.mColor), color3, 12);

            ImGui::NewLine();

            ImGui::SliderFloat("X", &dirLight.mDirection.x, -1.0f, 1.0f);
            ImGui::SliderFloat("Y", &dirLight.mDirection.y, -1.0f, 1.0f);
            ImGui::SliderFloat("Z", &dirLight.mDirection.z, -1.0f, 1.0f);

            ImGui::NewLine();

            ImGui::SliderFloat("Intensity", &dirLight.mIntensity, 0.1f, 20.0f);

            ImGui::End();
        }
    };
}
