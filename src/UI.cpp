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

#include "UI.hpp"
#include "Cameras.hpp"
#include "Poe.hpp"

namespace Poe
{
    ////////////////////////////////////////
    bool DebugUI::mEnableWireframe{false};
    bool DebugUI::mEnableSkybox{true};
    bool DebugUI::mEnableGrid{true};
    bool DebugUI::mEnableVsync{true};
    std::vector<std::string> DebugUI::mCoutLogs{};
    std::vector<std::string> DebugUI::mCerrLogs{};

    ////////////////////////////////////////
    void DebugUI::Init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450 core");

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("../fonts/Ubuntu-Regular.ttf", 18);
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    ////////////////////////////////////////
    void DebugUI::Destroy()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_General()
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

    ////////////////////////////////////////
    void DebugUI::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    ////////////////////////////////////////
    void DebugUI::NewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_Camera(FirstPersonCamera& camera)
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

    ////////////////////////////////////////
    void DebugUI::Begin_GlobalInfo()
    {
        ImGui::SetNextWindowSize({ 400, 0 });
        ImGui::SetNextWindowBgAlpha(BG_ALPHA);
        ImGui::Begin("Poe Global Info (OpenGL 4.5 Core)");
    }

    ////////////////////////////////////////
    void DebugUI::End_GlobalInfo()
    {
        ImGui::End();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_PostProcess(PostProcessProgram& program)
    {
        ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Post-Process]");
        ImGui::SliderFloat("Gamma", &program.mGamma, 0.1f, 5.0f);
        ImGui::SliderFloat("Exposure", &program.mExposure, 0.1f, 5.0f);
        ImGui::SliderFloat("Grayscale", &program.mGrayscaleWeight, 0.0f, 1.0f);
        ImGui::SliderFloat("Kernel", &program.mKernelWeight, 0.0f, 1.0f);
        ImGui::NewLine();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_Fog(FogUB& fogBlock)
    {
        ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Fog Settings]");
        float fogDistance = fogBlock.GetDistance();
        ImGui::SliderFloat("Distance", &fogDistance, 1.0f, 1000.0f);
        if (fogDistance != fogBlock.GetDistance())
            fogBlock.SetDistance(fogDistance);
        float fogExponent = fogBlock.GetExponent();
        ImGui::SliderFloat("Exponent", &fogExponent, 0.01f, 3.0f);
        if (fogExponent != fogBlock.GetExponent())
            fogBlock.SetExponent(fogExponent);
        glm::vec3 color = fogBlock.GetColor();
        float colorF[]{ color.x, color.y, color.z };
        ImGui::ColorEdit3("Color", colorF);
        color = glm::vec3(colorF[0], colorF[1], colorF[2]);
        if (color != fogBlock.GetColor())
            fogBlock.SetColor(color);
        ImGui::NewLine();
    }

    ////////////////////////////////////////
    void DebugUI::Render_LogInfo()
    {
        ImGui::SetNextWindowSize({ 800, 0 });
        ImGui::SetNextWindowBgAlpha(BG_ALPHA);

        ImGui::Begin("stdout");
        ImGui::BeginChild("stdout logs", { -1, 600 });
        for (int i = 0; i < MAX_COUT_LOGS && i < static_cast<int>(mCoutLogs.size()); ++i)
            ImGui::TextWrapped(mCoutLogs[i].c_str());
        ImGui::EndChild();
        ImGui::End();

        ImGui::SetNextWindowSize({ 800, 0 });
        ImGui::SetNextWindowBgAlpha(BG_ALPHA);

        ImGui::Begin("stderr");
        ImGui::BeginChild("stderr logs", { -1, 600 });
        for (int i = 0; i < MAX_CERR_LOGS && i < static_cast<int>(mCerrLogs.size()); ++i)
            ImGui::TextWrapped(mCerrLogs[i].c_str());
        ImGui::EndChild();
        ImGui::End();
    }

    ////////////////////////////////////////
    void DebugUI::PushLog(FILE* file, const char* format, ...)
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

    ////////////////////////////////////////
    void DebugUI::Render_EmissiveColorMaterialInfo(EmissiveColorMaterial& mat)
    {
        ImGui::SetNextWindowBgAlpha(BG_ALPHA);
        ImGui::Begin("Emissive Color Material");
        float color4[] { mat.mColor.x, mat.mColor.y, mat.mColor.z, mat.mColor.w };
        ImGui::ColorEdit3("Color", color4);
        std::memcpy(glm::value_ptr(mat.mColor), color4, 16);
        ImGui::End();
    }

    ////////////////////////////////////////
    void DebugUI::Render_DirLightInfo(DirLight& dirLight)
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

    ////////////////////////////////////////
    void DebugUI::Render_PbrLightMaterialInfo(PbrLightMaterial& material)
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
}
