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

#include "Poe.hpp"
#include "IO.hpp"
#include "UI.hpp"
#include "Utility.hpp"
#include "Cameras.hpp"
#include "Constants.hpp"
#include "Window.hpp"

#include <chrono>
#include <thread>
#include <utility>
#include <cstdio>
#include <cstdlib>

namespace CSItalyDemo
{
    ////////////////////////////////////////
    static Poe::FirstPersonCamera mainCamera;

    ////////////////////////////////////////
    static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
    {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
            }
        }
        mainCamera.UpdateInputConfig(key, action);
    }

    ////////////////////////////////////////
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
            static bool isCaptured;
            isCaptured = !isCaptured;
            if (isCaptured) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mainCamera.mIsMouseCaptured = true;
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mainCamera.mIsMouseCaptured = false;
            }
        }
    }

    ////////////////////////////////////////
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        mainCamera.UpdateDirection(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    ////////////////////////////////////////
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        mainCamera.mSpeed += static_cast<float>(yoffset) * 10.0f;
        mainCamera.mSpeed = glm::clamp(mainCamera.mSpeed, 1.0f, 500.0f);
    }

    ////////////////////////////////////////
    static void SetCallbacks(GLFWwindow* window)
    {
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetScrollCallback(window, scrollCallback);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    ////////////////////////////////////////
    static int Run(int argc, char** argv)
    {
        GLFWwindow* window = Poe::Window::CreateFullScreenWindow("Poe Renderer");
        SetCallbacks(window);

        Poe::DebugUI::Init(window);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        auto grid = Poe::CreateGrid(100, 100, 0);
        auto cube = Poe::CreateCube(0);

        mainCamera.mPosition = mainCamera.mTargetPosition = glm::vec3(-65.0f, -10.0f, 180.0f);

        glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), mainCamera.mPosition + glm::vec3(0.0f, 0.0f, -30.0f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));

        int numDirLights{ 2 }, numPointLights{ 4 }, numSpotLights{ 2 };
        int shadowSize{ 1024 };
        float directionalShadowMinBias{ 0.01f }, directionalShadowMaxBias{ 0.1f };
        float omniShadowBias{ 0.005f };
        constexpr int numCascades{ 4 };

        Poe::ShaderLoader shaderLoader;
        Poe::LightingStack<numCascades> lightingStack(numDirLights, numPointLights, numSpotLights, shadowSize, "..", shaderLoader);

        Poe::EmissiveColorProgram emissiveColorProgram("..", shaderLoader);
        Poe::RealisticSkyboxProgram skybox("..", shaderLoader);
        Poe::BlinnPhongProgram blinnPhongProgram("..",
                                                 shaderLoader,
                                                 numDirLights,
                                                 numPointLights,
                                                 numSpotLights,
                                                 numCascades,
                                                 directionalShadowMinBias,
                                                 directionalShadowMaxBias,
                                                 omniShadowBias);

        Poe::Texture2DLoader texture2DLoader;
        Poe::StaticModel staticModel = LoadCsItaly("..", texture2DLoader);
        std::vector<std::reference_wrapper<const Poe::StaticMesh>> staticModelMeshList = staticModel.ExtractMeshes();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f));

        Poe::PostProcessStack ppStack("..", fbWidth, fbHeight, shaderLoader);
        mainCamera.SetAspectRatio(ppStack.GetWidth(), ppStack.GetHeight());

        Poe::FogUB fogBlock(glm::vec3(1.0f), 1000.0f, 2.0f);
        fogBlock.Buffer().TurnOn();

        Poe::TransformUB transformBlock;
        transformBlock.Buffer().TurnOn();

        Poe::EmissiveColorMaterial gridMaterial{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
        Poe::EmissiveColorMaterial cubeMaterial{ glm::vec4(0.25f, 0.5f, 1.0f, 1.0f) };

        Poe::BlinnPhongMaterialUB blinnPhongBlock;
        blinnPhongBlock.Buffer().TurnOn();

        Poe::BlinnPhongMaterial blinnPhongMaterial{ glm::vec3(1.0f, 1.0f, 1.0f),
                                                    glm::vec3(1.0f, 1.0f, 1.0f),
                                                    glm::vec3(1.0f, 1.0f, 1.0f),
                                                    32.0f };
        blinnPhongBlock.Set(blinnPhongMaterial);
        blinnPhongBlock.Update();

        Poe::DirLight sun{
            glm::vec3(1.0f, 1.0f, 1.0f),    // color
            glm::vec3(0.0f, 0.0f, -1.0f),   // direction
            1.0f,                           // intensity,
            1000.0f,                        // far plane
            { 50.0f, 100.0f, 250.0f, 500.0f }, // cascades
            std::vector<glm::mat4>(numCascades + 1),       // light matrix
            true,                           // cast shadows
            10.0f,                          // z-offset
            10.0f                           // z-multiplier
        };

        Poe::PointLight playerLight{
            glm::vec3(1.0f, 1.0f, 0.0f),    // color
            glm::vec3(0.0f),                // world position
            glm::vec3(0.0f),                // view position
            50.0f,                          // radius
            10.0f,                          // intensity
            false,                          // cast shadows,
            0.3f,                           // near plane
            200.0f                          // far plane
        };

        Poe::SpotLight flashlight{
            glm::vec3(1.0f),                // color
            glm::vec3(0.0f),                // direction
            glm::vec3(0.0f),                // position
            glm::cos(glm::radians(20.0f)),  // inner cutoff
            glm::cos(glm::radians(30.0f)),  // outer cutoff
            20.0f,                          // radius
            10.0f,                          // intensity
            glm::mat4(1.0f),                // light matrix
            false                           // cast shadows
        };

        Poe::RealisticSkyboxUB skyboxBlock;
        skyboxBlock.Buffer().TurnOn();

        float ambientFactor{0.1f};

        float totalDt{};
        while (!glfwWindowShouldClose(window)) {

            float dt = Poe::Utility::ComputeDeltaTime();
            totalDt += dt;

            mainCamera.Update(dt);

            transformBlock.Set(mainCamera);
            transformBlock.Update();
            fogBlock.Update();

            sun.mDirection = glm::normalize(-skyboxBlock.GetSunPosition());
            sun.mIntensity = glm::max(0.0f, skyboxBlock.GetSunIntensity() * glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(skyboxBlock.GetSunPosition())));

            playerLight.mWorldPosition = mainCamera.mPosition;
            playerLight.mViewPosition = mainCamera.GetViewMatrix() * glm::vec4(mainCamera.mPosition, 1.0f);

            flashlight.mPosition = mainCamera.mPosition;
            flashlight.mDirection = mainCamera.mDirection;

            blinnPhongBlock.Set(blinnPhongMaterial);
            blinnPhongBlock.Update();

            lightingStack.PrepareState();
            lightingStack.DirectionalShadowPrepass(mainCamera, { sun }, { model }, staticModelMeshList);
            lightingStack.OmnidirectionalShadowPrepass({ playerLight }, { model }, staticModelMeshList);
            lightingStack.PerspectiveShadowPrepass({ flashlight }, { model }, staticModelMeshList);
            lightingStack.ResetState();

            ppStack.FirstPass();

            if (Poe::DebugUI::mEnableWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_CULL_FACE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_CULL_FACE);
            }

            if (Poe::DebugUI::mEnableVsync)
                glfwSwapInterval(1);
            else
                glfwSwapInterval(0);

            glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(mainCamera.GetViewMatrix() * model)));

            blinnPhongProgram.Use();
            blinnPhongProgram.SetModelMatrix(model);
            blinnPhongProgram.SetNormalMatrix(normal);
            blinnPhongProgram.SetAmbientFactor(ambientFactor);
            blinnPhongProgram.SetTexMultiplier(glm::vec2(1.0f));
            blinnPhongProgram.SetTexOffset(glm::vec2(0.0f));
            staticModel.Draw();

            emissiveColorProgram.Use();
            emissiveColorProgram.SetMaterial(cubeMaterial);
            emissiveColorProgram.SetModelMatrix(cubeModel * glm::rotate(glm::mat4(1.0f), totalDt, glm::vec3(1.0f)));
            cube.Bind();
            cube.Draw();

            if (Poe::DebugUI::mEnableGrid) {
                emissiveColorProgram.SetMaterial(gridMaterial);
                emissiveColorProgram.SetModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)));
                grid.Bind();
                grid.Draw(GL_LINES);
            }

            if (Poe::DebugUI::mEnableSkybox) {
                skybox.Draw();
            }

            ppStack.SecondPass();
            ppStack.BindColor0();

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);

            ppStack.Use();
            ppStack.Draw();

            Poe::DebugUI::NewFrame();
            Poe::DebugUI::Begin_GlobalInfo();
                Poe::DebugUI::Draw_GlobalInfo_General();
                Poe::DebugUI::Draw_GlobalInfo_Camera(mainCamera);
                Poe::DebugUI::Draw_GlobalInfo_PostProcess(ppStack.GetBlock());
                Poe::DebugUI::Draw_GlobalInfo_Fog(fogBlock);
                Poe::DebugUI::Draw_GlobalIlluminationInfo(ambientFactor);
            Poe::DebugUI::End_GlobalInfo();

            Poe::DebugUI::Render_SkyboxInfo(skyboxBlock, fbWidth, fbHeight);
            Poe::DebugUI::Render_LogInfo(fbWidth, fbHeight);
            Poe::DebugUI::Render_DirectionalLightsInfo({ sun }, fbWidth, fbHeight);
            Poe::DebugUI::Render_PointLightsInfo({ playerLight }, fbWidth, fbHeight);
            Poe::DebugUI::Render_SpotLightsInfo({ flashlight }, fbWidth, fbHeight);
            Poe::DebugUI::Render_BlinnPhongMaterialsInfo({ blinnPhongMaterial }, fbWidth, fbHeight);
            Poe::DebugUI::RenderStats(fbWidth, fbHeight, 75.0f);
            Poe::DebugUI::EndFrame();

            Poe::RuntimeStats::Reset();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        Poe::DebugUI::Destroy();
        glfwTerminate();
        return EXIT_SUCCESS;
    }
}

////////////////////////////////////////
int main(int argc, char** argv)
{
    return CSItalyDemo::Run(argc, argv);
}
