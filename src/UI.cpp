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

namespace Poe
{
    ////////////////////////////////////////
    bool DebugUI::mEnableWireframe{false};
    bool DebugUI::mEnableSkybox{true};
    float DebugUI::mGamma{2.2f};
    float DebugUI::mExposure{1.0f};
    float DebugUI::mGrayscaleWeight{0.0f};
    float DebugUI::mKernelWeight{0.0f};

    ////////////////////////////////////////
    void DebugUI::Init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450 core");

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("../fonts/VT323-Regular.ttf", 22);
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
        ImGui::SliderFloat("Speed", &camera.mSpeed, 1.0f, 200.0f);
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
        ImGui::SetNextWindowPos({ 50, 50 });
        ImGui::SetNextWindowSize({ 400, 0 });
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Poe Global Info (OpenGL 4.5 Core)");
    }

    ////////////////////////////////////////
    void DebugUI::End_GlobalInfo()
    {
        ImGui::End();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_PostProcess()
    {
        ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Post-Process]");
        ImGui::SliderFloat("Gamma", &mGamma, 0.1f, 5.0f);
        ImGui::SliderFloat("Exposure", &mExposure, 0.1f, 5.0f);
        ImGui::SliderFloat("Grayscale", &mGrayscaleWeight, 0.0f, 1.0f);
        ImGui::SliderFloat("Kernel", &mKernelWeight, 0.0f, 1.0f);
        ImGui::NewLine();
    }

    ////////////////////////////////////////
    void DebugUI::Draw_GlobalInfo_Fog(FogUB& fogBlock)
    {
        ImGui::TextColored({ 0.0f, 1.0f, 1.0f, 1.0f }, "[Fog Settings]");
        float fogDistance = fogBlock.GetDistance();
        ImGui::SliderFloat("Distance", &fogDistance, 1.0f, 500.0f);
        if (fogDistance != fogBlock.GetDistance())
            fogBlock.SetDistance(fogDistance);
        float fogExponent = fogBlock.GetExponent();
        ImGui::SliderFloat("Exponent", &fogExponent, 1.0f, 5.0f);
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
}
