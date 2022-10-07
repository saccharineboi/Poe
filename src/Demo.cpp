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
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            static bool isWireframe;
            isWireframe = !isWireframe;
            if (isWireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        constexpr glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

        auto grid = CreateGrid(100, 100);

        ShaderLoader shaderLoader;
        auto emissiveColorProgram = CreateEmissiveColorProgram("..", shaderLoader);
        auto emissiveTextureProgram = CreateEmissiveTextureProgram("..", shaderLoader);
        auto skyboxProgram = CreateTextureSkyboxProgram("..", shaderLoader);

        mainCamera.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));

        Texture2DLoader texture2DLoader;
        StaticModel staticModel("../../../Desktop/FreeModels/cs_italy/cs_italy.obj", texture2DLoader);

        auto fboTexture = CreateFramebufferTexture2D(fbWidth, fbHeight);
        Renderbuffer rbo(GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
        Framebuffer fbo(fboTexture, rbo);

        PostProcessProgram postProcessProgram("..", shaderLoader);

        Cubemap cubemap{
            std::make_pair(CubemapFace::Front, "../skyboxes/ulukai/corona_ft.png"),
            std::make_pair(CubemapFace::Back, "../skyboxes/ulukai/corona_bk.png"),
            std::make_pair(CubemapFace::Left, "../skyboxes/ulukai/corona_lf.png"),
            std::make_pair(CubemapFace::Right, "../skyboxes/ulukai/corona_rt.png"),
            std::make_pair(CubemapFace::Top, "../skyboxes/ulukai/corona_up.png"),
            std::make_pair(CubemapFace::Bottom, "../skyboxes/ulukai/corona_dn.png")
        };

        float rads = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            fbo.Bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            float dt = Utility::ComputeDeltaTime();
            mainCamera.Update(dt);
            auto projView = mainCamera.mProjection * mainCamera.mView;

            rads += dt;

            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.01f));

            emissiveTextureProgram.Use();
            emissiveTextureProgram.Uniform("uPVM", projView * model);
            emissiveTextureProgram.Uniform("uModelView", mainCamera.mView * model);
            emissiveTextureProgram.Uniform("uFogColor", clearColor);
            emissiveTextureProgram.Uniform("uFogDistance", 1000.0f);
            emissiveTextureProgram.Uniform("uFogExp", 3.0f);
            emissiveTextureProgram.Uniform("uTileMultiplier", glm::vec2(1.0f));
            emissiveTextureProgram.Uniform("uTileOffset", glm::vec2(0.0f));
            staticModel.Draw();

            emissiveColorProgram.Use();
            emissiveColorProgram.Uniform("uPVM", projView);
            emissiveColorProgram.Uniform("uModelView", mainCamera.mView);
            emissiveColorProgram.Uniform("uColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
            emissiveColorProgram.Uniform("uFogColor", clearColor);
            emissiveColorProgram.Uniform("uFogDistance", 1000.0f);
            emissiveColorProgram.Uniform("uFogExp", 3.0f);
            grid.Bind();
            grid.Draw(GL_LINES);

            skyboxProgram.Use();
            skyboxProgram.Uniform("uProjView", mainCamera.mProjection * glm::mat4(glm::mat3(mainCamera.mView)));
            cubemap.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);

            fbo.UnBind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            fboTexture.Bind();
            postProcessProgram.Use();
            postProcessProgram.SetGrayscaleWeight(0.0f);
            postProcessProgram.SetKernelWeight(0.0f);
            postProcessProgram.SetIdentityKernel();
            postProcessProgram.Draw();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return EXIT_SUCCESS;
    }
}
