// Poe: OpenGL 3.3 Renderer
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

#include <cstdio>
#include <cstdlib>

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
        GLFWwindow* window = glfwCreateWindow(1600, 900, "Poe", nullptr, nullptr);
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
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
        std::printf("[DEBUG] GL version: %s\n", glGetString(GL_VERSION));
        std::printf("[DEBUG] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        std::printf("[DEBUG] GL renderer: %s\n", glGetString(GL_RENDERER));
        std::printf("[DEBUG] GL vendor: %s\n", glGetString(GL_VENDOR));
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
    int Run()
    {
        InitGLFW();
        SetHints();
        GLFWwindow* window = CreateWindow();
        InitOpenGL(window);
        DebugOutput();
        EnableDebugContext();
        SetCallbacks(window);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        constexpr glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

        auto staticMesh = CreateCube();
        auto grid = CreateGrid(100, 100);

        ShaderLoader shaderLoader;
        auto emissiveColorProgram = CreateEmissiveColorProgram("../shaders/", shaderLoader);
        auto emissiveTextureProgram = CreateEmissiveTextureProgram("../shaders/", shaderLoader);

        mainCamera.mPosition = glm::vec3(0.0f, 3.0f, 3.0f);
        mainCamera.mTargetPosition = mainCamera.mPosition;

        // Texture2DLoader texture2DLoader;
        // Texture2D& meshTexture = texture2DLoader.Load("../textures/abstract_5-4K/4K-abstract_5-diffuse.jpg", Texture2DParams{});

        auto meshTexture = CreateCheckerboardTexture2D();

        meshTexture.Bind();

        float rads = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float dt = Utility::ComputeDeltaTime();
            mainCamera.Update(dt);

            rads += dt;
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
            model = glm::rotate(model, rads, glm::vec3(1.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(2.0f));

            emissiveTextureProgram.Use();
            emissiveTextureProgram.Uniform("uPVM", mainCamera.mProjection * mainCamera.mView * model);
            emissiveTextureProgram.Uniform("uModelView", mainCamera.mView * model);
            emissiveTextureProgram.Uniform("uFogColor", clearColor);
            emissiveTextureProgram.Uniform("uFogDistance", 25.0f);
            emissiveTextureProgram.Uniform("uFogExp", 3.0f);
            emissiveTextureProgram.Uniform("uTileMultiplier", glm::vec2(3.0f));
            emissiveTextureProgram.Uniform("uTileOffset", glm::vec2(0.0f));

            staticMesh.Bind();
            staticMesh.Draw();

            emissiveColorProgram.Use();
            emissiveColorProgram.Uniform("uPVM", mainCamera.mProjection * mainCamera.mView);
            emissiveColorProgram.Uniform("uModelView", mainCamera.mView);
            emissiveColorProgram.Uniform("uColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
            emissiveColorProgram.Uniform("uFogColor", clearColor);
            emissiveColorProgram.Uniform("uFogDistance", 25.0f);
            emissiveColorProgram.Uniform("uFogExp", 3.0f);
            grid.Bind();
            grid.Draw(GL_LINES);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return EXIT_SUCCESS;
    }
}
