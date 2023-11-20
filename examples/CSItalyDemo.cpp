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
    static void InitGLFW()
    {
        if (!glfwInit()) {
            std::fprintf(stderr, "ERROR: couldn't initialize GLFW\n");
            std::exit(EXIT_FAILURE);
        }
    }

    ////////////////////////////////////////
    static GLFWwindow* CreateWindow()
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            std::fprintf(stderr, "ERROR: couldn't get primary monitor\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) {
            std::fprintf(stderr, "ERROR: couldn't get video mode\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Poe", monitor, nullptr);
        if (!window) {
            std::fprintf(stderr, "ERROR: couldn't create window\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        return window;
    }

    ////////////////////////////////////////
    static void SetHints()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    }

    ////////////////////////////////////////
    static void InitOpenGL(GLFWwindow* window)
    {
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            std::fprintf(stderr, "ERROR: couldn't initialize glad\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
    }

    ////////////////////////////////////////
    static void DebugOutput()
    {
        Poe::DebugUI::PushLog(stdout, "[DEBUG] GL version: %s\n", glGetString(GL_VERSION));
        Poe::DebugUI::PushLog(stdout, "[DEBUG] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        Poe::DebugUI::PushLog(stdout, "[DEBUG] GL renderer: %s\n", glGetString(GL_RENDERER));
        Poe::DebugUI::PushLog(stdout, "[DEBUG] GL vendor: %s\n", glGetString(GL_VENDOR));
    }

    ////////////////////////////////////////
    static void EnableDebugContext()
    {
#ifdef _DEBUG
        int dflags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &dflags);
        if (dflags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(Poe::GraphicsDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            Poe::DebugUI::PushLog(stdout, "[DEBUG] GL debug output is ON\n");
        }
        else
            Poe::DebugUI::PushLog(stdout, "[DEBUG] GL debug output is OFF\n");
#endif
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
        InitGLFW();
        SetHints();
        GLFWwindow* window = CreateWindow();
        InitOpenGL(window);
        DebugOutput();
        EnableDebugContext();
        SetCallbacks(window);

        Poe::DebugUI::Init(window);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        auto grid = Poe::CreateGrid(100, 100, 0);

        Poe::ShaderLoader shaderLoader;
        Poe::EmissiveColorProgram emissiveColorProgram("..", shaderLoader);
        Poe::RealisticSkyboxProgram skybox("..", shaderLoader);
        Poe::BlinnPhongProgram blinnPhongProgram("..", shaderLoader);
        Poe::DepthProgram depthProgram("..", shaderLoader);

        mainCamera.SetPosition(glm::vec3(-65.0f, -10.0f, 180.0f));

        Poe::Texture2DLoader texture2DLoader;
        auto staticModel = LoadCsItaly("..", texture2DLoader);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f));

        constexpr int fbSizeMultiplier{ 1 };
        Poe::PostProcessStack ppStack("..", fbWidth / fbSizeMultiplier, fbHeight / fbSizeMultiplier, fbWidth, fbHeight, 8, shaderLoader);
        mainCamera.SetAspectRatio(ppStack.GetWidth(), ppStack.GetHeight());

        Poe::PostProcessUB ppBlock;
        ppBlock.SetExposure(1.0f);
        ppBlock.SetGamma(2.2f);
        ppBlock.Buffer().TurnOn();

        Poe::FogUB fogBlock(glm::vec3(0.01f, 0.01f, 0.01f), 1000.0f, 2.0f);
        fogBlock.Buffer().TurnOn();

        Poe::TransformUB transformBlock;
        transformBlock.Buffer().TurnOn();

        Poe::EmissiveColorMaterial gridMaterial{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

        Poe::DirLightUB dirLightBlock;
        dirLightBlock.Buffer().TurnOn();

        Poe::PointLightUB pointLightBlock;
        pointLightBlock.Buffer().TurnOn();

        Poe::SpotLightUB spotLightBlock;
        spotLightBlock.Buffer().TurnOn();

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
            glm::mat4(1.0f),                // light matrix
            true                            // cast shadows
        };

        Poe::PointLight playerLight{
            glm::vec3(1.0f, 1.0f, 0.0f),    // color
            glm::vec3(0.0f),                // position
            50.0f,                          // radius
            10.0f,                          // intensity
            glm::mat4(1.0f),                // light matrix
            false                           // cast shadows
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

        Poe::Texture2D dirLightDepthMap = Poe::CreateDepthMap(2048, 2048);
        Poe::Framebuffer dirLightDepthFBO(dirLightDepthMap, GL_DEPTH_ATTACHMENT);

        Poe::Cubemap pointLightDepthMap = Poe::CreateDepthCubemap(2048, 2048);
        Poe::Framebuffer pointLightDepthFBO(pointLightDepthMap, GL_DEPTH_ATTACHMENT);;

        while (!glfwWindowShouldClose(window)) {
            
            depthProgram.Use();
            sun.mLightMatrix = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, -200.0f, 200.0f) *
                               glm::lookAt(skyboxBlock.GetSunPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glViewport(0, 0, dirLightDepthMap.GetWidth(), dirLightDepthMap.GetHeight());
            dirLightDepthFBO.Bind();
                glDisable(GL_CULL_FACE);
                glClear(GL_DEPTH_BUFFER_BIT);
                depthProgram.SetModelMatrix(model);
                depthProgram.SetLightMatrix(sun.mLightMatrix);
                staticModel.Draw();
                glEnable(GL_CULL_FACE);
            dirLightDepthFBO.UnBind();
            dirLightDepthMap.Bind(Poe::DIR_LIGHT_DEPTH_MAP_BIND_POINT);

            {
                glm::mat4 shadowPerspective = glm::perspective(glm::radians(90.0f),
                                                               static_cast<float>(pointLightDepthMap.GetWidth()) / static_cast<float>(pointLightDepthMap.GetHeight()),
                                                               1.0f, playerLight.mRadius);

                std::vector<glm::mat4> viewTransforms{ glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                                       glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                                       glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                                                       glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                                                       glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                                       glm::lookAt(playerLight.mPosition, playerLight.mPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) };
            }

            ppStack.FirstPass();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

            float dt = Poe::Utility::ComputeDeltaTime();
            mainCamera.Update(dt);

            transformBlock.Set(mainCamera);
            transformBlock.Update();
            fogBlock.Update();

            sun.mDirection = glm::normalize(-skyboxBlock.GetSunPosition());
            sun.mIntensity = glm::max(0.0f, skyboxBlock.GetSunIntensity() * glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(skyboxBlock.GetSunPosition())));
            dirLightBlock.Set(0, mainCamera.mView, sun);
            dirLightBlock.Update();

            playerLight.mPosition = mainCamera.mPosition;
            pointLightBlock.Set(0, mainCamera.mView, playerLight);
            pointLightBlock.Update();

            flashlight.mPosition = mainCamera.mPosition;
            flashlight.mDirection = mainCamera.mDirection;
            spotLightBlock.Set(0, mainCamera.mView, flashlight);
            spotLightBlock.Update();

            blinnPhongBlock.Set(blinnPhongMaterial);
            blinnPhongBlock.Update();

            glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(mainCamera.mView * model))); 

            blinnPhongProgram.Use();
            blinnPhongProgram.SetModelMatrix(model);
            blinnPhongProgram.SetNormalMatrix(normal);
            blinnPhongProgram.SetAmbientFactor(ambientFactor);
            blinnPhongProgram.SetTexMultiplier(glm::vec2(1.0f));
            blinnPhongProgram.SetTexOffset(glm::vec2(0.0f));
            staticModel.Draw();

            if (Poe::DebugUI::mEnableGrid) {
                emissiveColorProgram.Use();
                emissiveColorProgram.SetMaterial(gridMaterial);
                emissiveColorProgram.SetModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)));
                grid.Bind();
                grid.Draw(GL_LINES);
            }

            if (Poe::DebugUI::mEnableSkybox) {
                skybox.Draw();
            }

            ppStack.SecondPass();
            ppStack.BindColor0();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);

            ppStack.Use();
            ppStack.Draw();

            Poe::DebugUI::NewFrame();
            Poe::DebugUI::Begin_GlobalInfo();
                Poe::DebugUI::Draw_GlobalInfo_General();
                Poe::DebugUI::Draw_GlobalInfo_Camera(mainCamera);
                Poe::DebugUI::Draw_GlobalInfo_PostProcess(ppBlock);
                Poe::DebugUI::Draw_GlobalInfo_Fog(fogBlock);
                Poe::DebugUI::Draw_GlobalIlluminationInfo(ambientFactor);
                Poe::DebugUI::Render_DirLightInfo(sun);
                Poe::DebugUI::Render_PointLightInfo(playerLight);
                Poe::DebugUI::Render_SpotLightInfo(flashlight);
                Poe::DebugUI::Render_SkyboxInfo(skyboxBlock);
                Poe::DebugUI::Render_BlinnPhongMaterialInfo(blinnPhongMaterial);
            Poe::DebugUI::End_GlobalInfo();
            Poe::DebugUI::Render_LogInfo();

            Poe::DebugUI::EndFrame();

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
