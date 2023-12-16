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

        inline static constexpr glm::vec4 HEADER_COLOR{ 0.0f, 1.0f, 1.0f, 1.0f };

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
            ImGui::SetNextWindowPos({ 20, 20 });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);

            GLint glVersionMajor{}, glVersionMinor{};
            glGetIntegerv(GL_MAJOR_VERSION, &glVersionMajor);
            glGetIntegerv(GL_MINOR_VERSION, &glVersionMinor);

            ImGui::Begin("Poe Global Info");
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
            ImGui::TextColored({ HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, HEADER_COLOR.a }, "[General]");
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
            ImGui::TextColored({ HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, HEADER_COLOR.a }, "[Camera]");
            ImGui::InputFloat3("Position", glm::value_ptr(camera.mTargetPosition), "%.2f");
            ImGui::InputFloat3("Direction", glm::value_ptr(camera.mDirection), "%.2f");
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
            ImGui::TextColored({ HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, HEADER_COLOR.a }, "[Post-Process]");

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
            ImGui::TextColored({ HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, HEADER_COLOR.a }, "[Fog Settings]");
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

        static void Draw_GlobalIlluminationInfo(float& ambientFactor)
        {
            ImGui::TextColored({ HEADER_COLOR.r, HEADER_COLOR.g, HEADER_COLOR.b, HEADER_COLOR.a }, "[Global Illumination]");
            ImGui::SliderFloat("Ambient Factor", &ambientFactor, 0.0f, 1.0f);
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

        static void Render_LogInfo(int width, int height)
        {
            constexpr int coutWidth{ 400 }, cerrWidth{ 600 };
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);

            if (mCoutLogs.size() > 0)
            {
                ImGui::SetNextWindowSize({ coutWidth, -1 });
                ImGui::SetNextWindowPos({ static_cast<float>(width - coutWidth - 20), 60.0f });

                ImGui::Begin("Info Logs", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize);
                ImGui::BeginChild("stdout logs", { -1, 400 });
                for (size_t i = 0; i < MAX_COUT_LOGS && i < mCoutLogs.size(); ++i)
                    ImGui::TextWrapped("%s", mCoutLogs[i].c_str());
                ImGui::EndChild();
                ImGui::End();
            }

            if (mCerrLogs.size() > 0)
            {
                ImGui::SetNextWindowSize({ cerrWidth, -1 });
                ImGui::SetNextWindowPos({ static_cast<float>(width / 2 - cerrWidth / 2), 20.0f });

                ImGui::Begin("Error Logs", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize);
                ImGui::BeginChild("stderr logs", { -1, 400 });
                for (size_t i = 0; i < MAX_CERR_LOGS && i < mCerrLogs.size(); ++i)
                    ImGui::TextWrapped("%s", mCerrLogs[i].c_str());
                ImGui::EndChild();
                ImGui::End();
            }
        }

        static void Render_EmissiveColorMaterialInfo(EmissiveColorMaterial& mat)
        {
            ImGui::SetNextWindowSize({ 300, 150 });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Emissive Color Material", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::ColorEdit3("Color", glm::value_ptr(mat.mColor));
            ImGui::End();
        }

        static void Render_PbrLightMaterialInfo(PbrLightMaterial& material)
        {
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("PBR Light Material");

            ImGui::ColorEdit3("Albedo", glm::value_ptr(material.mAlbedo));

            ImGui::NewLine();

            ImGui::SliderFloat("Metallic", &material.mMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &material.mRoughness, 0.0f, 1.0f);
            ImGui::SliderFloat("AO", &material.mAo, 0.0f, 1.0f);

            ImGui::End();
        }

        static void Render_SkyboxInfo(RealisticSkyboxUB& block, int width, int height)
        {
            constexpr int wwidth{ 400 }, wheight{ 330 };

            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::SetNextWindowPos({ static_cast<float>(width - wwidth - 20), 20 });
            ImGui::SetNextWindowSize({ wwidth, wheight });
            ImGui::Begin("Atmospheric Scattering", nullptr, ImGuiWindowFlags_NoResize |
                                                            ImGuiWindowFlags_HorizontalScrollbar);

            RealisticSkyboxMaterial material{ block.Get() };

            ImGui::InputFloat3("Ray Origin", glm::value_ptr(material.mRayOrigin), "%.2f");
            ImGui::InputFloat3("Sun Position", glm::value_ptr(material.mSunPosition), "%.2f");
            ImGui::SliderFloat("Sun Intensity", &material.mSunIntensity, 0.0f, 100.0f);
            ImGui::InputFloat("Planet Radius", &material.mPlanetRadius);
            ImGui::InputFloat("Atmosphere Radius", &material.mAtmosphereRadius);
            ImGui::InputFloat3("Rayleigh Scattering Coefficient", glm::value_ptr(material.mRayleighScatteringCoefficient), "%.2f");
            ImGui::InputFloat("Mie Scattering Coefficient", &material.mMieScatteringCoefficient);
            ImGui::InputFloat("Rayleigh Scale Height", &material.mRayleighScaleHeight);
            ImGui::InputFloat("Mie Scale Height", &material.mMieScaleHeight);
            ImGui::InputFloat("Mie Preferred Scattering Direction", &material.mMiePreferredScatteringDirection);

            block.Set(material);
            block.Update();
            ImGui::End();
        }

        static void Render_DirectionalLightsInfo(const std::vector<std::reference_wrapper<DirLight>>& dirLights, int width, int height)
        {
            constexpr int wwidth{ 400 };
            ImGui::SetNextWindowSize({ wwidth, -1 });
            ImGui::SetNextWindowPos({ static_cast<float>(width - wwidth - 20), 100.0f });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Directional Lights", nullptr, ImGuiWindowFlags_NoResize);

            int dirLightIndex{};
            for (DirLight& light : dirLights) {
                ImGui::Text("Light #%d", dirLightIndex);

                ImGui::ColorEdit3("Color", glm::value_ptr(light.mColor));
                ImGui::Text("Direction: %.2f %.2f %.2f", light.mDirection.x, light.mDirection.y, light.mDirection.z);
                ImGui::Text("Intensity: %.2f", light.mIntensity);
                ImGui::Checkbox("Cast Shadows", &light.mCastShadows);
                ImGui::InputScalarN("Cascades", ImGuiDataType_Float, light.mCascadeRanges.data(), static_cast<int>(light.mCascadeRanges.size()));

                ++dirLightIndex;

                if (dirLightIndex != static_cast<int>(dirLights.size())) {
                    ImGui::NewLine();
                }
            }
            ImGui::End();
        }

        static void Render_PointLightsInfo(const std::vector<std::reference_wrapper<PointLight>>& pointLights, int width, int height)
        {
            constexpr int wwidth{ 400 };
            ImGui::SetNextWindowSize({ wwidth, -1 });
            ImGui::SetNextWindowPos({ static_cast<float>(width - wwidth - 20), 140.0f });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Point Lights", nullptr, ImGuiWindowFlags_NoResize);

            int pointLightIndex{};
            for (PointLight& light : pointLights) {
                ImGui::Text("Light #%d", pointLightIndex);

                ImGui::ColorEdit3("Color", glm::value_ptr(light.mColor));
                ImGui::Text("World Pos: %.2f %.2f %.2f", light.mWorldPosition.x, light.mWorldPosition.y, light.mWorldPosition.z);
                ImGui::Text("View Pos: %.2f %.2f %.2f", light.mViewPosition.x, light.mViewPosition.y, light.mViewPosition.z);
                ImGui::SliderFloat("Intensity", &light.mIntensity, 0.0f, 100.0f);
                ImGui::SliderFloat("Radius", &light.mRadius, 0.0f, 100.0f);
                ImGui::Checkbox("Cast Shadows", &light.mCastShadows);

                ++pointLightIndex;

                if (pointLightIndex != static_cast<int>(pointLights.size())) {
                    ImGui::NewLine();
                }
            }
            ImGui::End();
        }

        static void Render_SpotLightsInfo(const std::vector<std::reference_wrapper<SpotLight>>& spotLights, int width, int height)
        {
            constexpr int wwidth{ 400 };
            ImGui::SetNextWindowSize({ wwidth, -1 });
            ImGui::SetNextWindowPos({ static_cast<float>(width - wwidth - 20), 180.0f });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);
            ImGui::Begin("Spot Lights", nullptr, ImGuiWindowFlags_NoResize);

            int spotLightIndex{};
            for (SpotLight& light : spotLights) {
                ImGui::Text("Light #%d", spotLightIndex);

                ImGui::ColorEdit3("Color", glm::value_ptr(light.mColor));
                ImGui::Text("Direction: %.2f %.2f %.2f", light.mDirection.x, light.mDirection.y, light.mDirection.z);
                ImGui::Text("Position: %.2f %.2f %.2f", light.mPosition.x, light.mPosition.y, light.mPosition.z);
                ImGui::SliderFloat("Intensity", &light.mIntensity, 0.0f, 100.0f);
                ImGui::SliderFloat("Inner Cutoff", &light.mInnerCutoff, 0.0f, PI);
                ImGui::SliderFloat("Outer Cutoff", &light.mOuterCutoff, 0.0f, PI);
                ImGui::SliderFloat("Radius", &light.mRadius, 0.0f, 100.0f);
                ImGui::Checkbox("Cast Shadows", &light.mCastShadows);

                ++spotLightIndex;

                if (spotLightIndex != static_cast<int>(spotLights.size())) {
                    ImGui::NewLine();
                }
            }
            ImGui::End();
        }

        static void Render_BlinnPhongMaterialsInfo(const std::vector<std::reference_wrapper<BlinnPhongMaterial>>& materials, int width, int height)
        {
            constexpr int wwidth{ 400 };
            ImGui::SetNextWindowSize({ wwidth, -1 });
            ImGui::SetNextWindowPos({ static_cast<float>(width - wwidth - 20), 220.0f });
            ImGui::SetNextWindowBgAlpha(BG_ALPHA);

            ImGui::Begin("Blinn-Phong Materials", nullptr, ImGuiWindowFlags_NoResize);

            int materialIndex{};
            for (BlinnPhongMaterial& material : materials) {
                ImGui::Text("Material #%d", materialIndex);

                ImGui::ColorEdit3("Ambient Color", glm::value_ptr(material.mAmbient));
                ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(material.mDiffuse));
                ImGui::ColorEdit3("Specular Color", glm::value_ptr(material.mSpecular));
                ImGui::InputFloat("Shininess", &material.mShininess);

                ++materialIndex;

                if (materialIndex != static_cast<int>(materials.size())) {
                    ImGui::NewLine();
                }
            }

            ImGui::End();
        }

        static void RenderStats(int width, int height, int thickness)
        {
            ImGui::SetNextWindowSize({ static_cast<float>(width), static_cast<float>(thickness) });
            ImGui::SetNextWindowPos({ 0.0f, static_cast<float>(height - thickness) });
            ImGui::SetNextWindowBgAlpha(1.0f);
            ImGui::Begin("No Title", nullptr, ImGuiWindowFlags_NoTitleBar |
                                              ImGuiWindowFlags_NoResize |
                                              ImGuiWindowFlags_NoDecoration);

            GLint numGLExtensions;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numGLExtensions);

            GLint flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            bool isRunningInDebugContext = flags & GL_CONTEXT_FLAG_DEBUG_BIT;

            ImGui::Text("GL Version: %s | GLSL Version: %s | GL Renderer: %s | GL Vendor: %s | Num GL Extensions: %d | Debug Mode: %d", glGetString(GL_VERSION),
                                                                                                                                        glGetString(GL_SHADING_LANGUAGE_VERSION),
                                                                                                                                        glGetString(GL_RENDERER),
                                                                                                                                        glGetString(GL_VENDOR),
                                                                                                                                        numGLExtensions,
                                                                                                                                        isRunningInDebugContext);

            if (GLAD_GL_ATI_meminfo) {
                constexpr GLenum VBO_FREE_MEMORY_ATI{ 0x87FB };
                constexpr GLenum TEXTURE_FREE_MEMORY_ATI{ 0x87FC };
                constexpr GLenum RENDERBUFFER_FREE_MEMORY_ATI{ 0x87FD };

                GLint vboMemory[4], textureMemory[4], renderbufferMemory[4];

                glGetIntegerv(VBO_FREE_MEMORY_ATI, vboMemory);
                glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, textureMemory);
                glGetIntegerv(RENDERBUFFER_FREE_MEMORY_ATI, renderbufferMemory);

                ImGui::Text("VBO: Total %d MB, Largest %d MB, Total Aux %d MB, Largest Aux %d MB | Texture: Total %d MB, Largest: %d MB, Total Aux: %d MB, Largest Aux: %d MB | Renderbuffer: Total %d MB, Largest: %d mb, Total Aux: %d MB, Largest Aux: %d MB", vboMemory[0] / 1000, vboMemory[1] / 1000, vboMemory[2] / 1000, vboMemory[3] / 1000, textureMemory[0] / 1000, textureMemory[1] / 1000, textureMemory[2] / 1000, textureMemory[3] / 1000, renderbufferMemory[0] / 1000, renderbufferMemory[1] / 1000, renderbufferMemory[2] / 1000, renderbufferMemory[3] / 1000);
            }
            ImGui::Text("# Draw Calls: %d | # Instanced Draw Calls: %d | # VAO Binds: %d | # Texture Binds: %d", RuntimeStats::NumDrawCalls, RuntimeStats::NumInstancedDrawCalls, RuntimeStats::NumVAOBinds, RuntimeStats::NumTextureBinds);

            ImGui::End();
        }
    };
}
