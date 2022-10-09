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

#include "Poe.hpp"
#include "IO.hpp"
#include "Utility.hpp"

#include <utility>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace Poe
{
    ////////////////////////////////////////
    static FirstPersonCamera mainCamera;

    ////////////////////////////////////////
    static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
    {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                case GLFW_KEY_I:
                    mainCamera.mFovy -= glm::radians(5.0f);
                    if (mainCamera.mFovy < glm::radians(10.0f))
                        mainCamera.mFovy = glm::radians(10.0f);
                    break;
                case GLFW_KEY_O:
                    mainCamera.mFovy += glm::radians(5.0f);
                    if (mainCamera.mFovy > glm::radians(180.0f))
                        mainCamera.mFovy = glm::radians(180.0f);
                    break;
            }
        }
        mainCamera.UpdateInputConfig(key, action);
    }

    ////////////////////////////////////////
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
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
        mainCamera.mSpeed += static_cast<float>(yoffset);
        mainCamera.mSpeed = glm::clamp(mainCamera.mSpeed, 1.0f, 1000.0f);
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
        return window;
    }

    ////////////////////////////////////////
    static void SetHints()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
#ifdef _DEBUG
        std::printf("[DEBUG] GL version: %s\n", glGetString(GL_VERSION));
        std::printf("[DEBUG] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        std::printf("[DEBUG] GL renderer: %s\n", glGetString(GL_RENDERER));
        std::printf("[DEBUG] GL vendor: %s\n", glGetString(GL_VENDOR));
#endif
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
            glDebugMessageCallback(GraphicsDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            std::printf("[DEBUG] GL debug output is ON\n");
        }
        else
            std::printf("[DEBUG] GL debug output is OFF\n");
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
    int Run(int argc, char** argv)
    {
        InitGLFW();
        SetHints();
        GLFWwindow* window = CreateWindow();
        InitOpenGL(window);
        DebugOutput();
        EnableDebugContext();
        SetCallbacks(window);

        int fbWidth = 1920, fbHeight = 1080;
        // glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        mainCamera.SetAspectRatio(fbWidth, fbHeight);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        constexpr glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

        auto cube = CreateCube();
        cube.CreateInstances(1000);

        auto grid = CreateGrid(100, 100);
        grid.SetInstanceMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));

        ShaderLoader shaderLoader;
        auto emissiveColorProgram = CreateEmissiveColorProgram("..", shaderLoader);
        auto emissiveTextureProgram = CreateEmissiveTextureProgram("..", shaderLoader);
        auto skyboxProgram = CreateTextureSkyboxProgram("..", shaderLoader);

        mainCamera.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));

        Texture2DLoader texture2DLoader;
        StaticModel staticModel("../../../Desktop/FreeModels/cs_italy/cs_italy.obj", texture2DLoader);

        PostProcessStack ppStack("..", fbWidth, fbHeight, 8, shaderLoader);

        auto cubemap = CreateCloudySkybox("..");

        FogUB fogBlock(clearColor, 100.0f, 2.0f);
        fogBlock.Buffer().TurnOn();

        TransformUB transformBlock(mainCamera.mProjection, mainCamera.mView);
        transformBlock.Buffer().TurnOn();

        float rads = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            ppStack.FirstPass();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            float dt = Utility::ComputeDeltaTime();
            mainCamera.Update(dt);

            transformBlock.SetProjectionMatrix(mainCamera.mProjection);
            transformBlock.SetViewMatrix(mainCamera.mView);

            auto projView = mainCamera.mProjection * mainCamera.mView;

            rads += dt;

            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.01f));

            emissiveTextureProgram.Use();
            emissiveTextureProgram.Uniform("uPVM", projView * model);
            emissiveTextureProgram.Uniform("uModelView", mainCamera.mView * model);
            emissiveTextureProgram.Uniform("uTileMultiplier", glm::vec2(1.0f));
            emissiveTextureProgram.Uniform("uTileOffset", glm::vec2(0.0f));
            staticModel.Draw();

            emissiveColorProgram.Use();
            emissiveColorProgram.Uniform("uColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
            grid.Bind();
            grid.Draw(GL_LINES);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 12.0f, -10.0f));
            model = glm::rotate(model, rads, glm::vec3(0.0f, 1.0f, 0.0f));

            emissiveColorProgram.Uniform("uColor", glm::vec4(0.25f, 0.5f, 1.0f, 1.0f));
            cube.Bind();

            cube.ApplyToAllInstancesGrid3D(10, 10, 10, 4.0f, 4.0f, 4.0f,
            [=](int i, int j, int k, int numInstances) {
                auto t = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 30.0f, -50.0f));
                t = glm::rotate(t, rads, glm::vec3(0.0f, 1.0f, 0.0f));
                return t;
            });

            cube.Draw();

            skyboxProgram.Use();
            skyboxProgram.Uniform("uProjView", mainCamera.mProjection * glm::mat4(glm::mat3(mainCamera.mView)));
            cubemap.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);

            ppStack.SecondPass();
            ppStack.BindColor0();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            ppStack.Program().Use();
            ppStack.Program().SetGrayscaleWeight(0.0f);
            ppStack.Program().SetKernelWeight(0.0f);
            ppStack.Program().SetGamma(2.2f);
            ppStack.Program().SetExposure(1.0f);
            ppStack.Program().Draw();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return EXIT_SUCCESS;
    }
}
