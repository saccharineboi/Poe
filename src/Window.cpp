// Poe: OpenGL 4.5 Renderer
// Copyright (C) 2024 Omar Huseynov
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

#include "Window.hpp"
#include "Constants.hpp"
#include "UI.hpp"

#include <cstdlib>
#include <cstdio>

namespace Poe::Window
{
    ////////////////////////////////////////
    static void InitGLFW()
    {
        if (!glfwInit()) {
            std::fprintf(stderr, "[ERROR] couldn't initialize GLFW\n");
            std::exit(EXIT_FAILURE);
        }
    }

    ////////////////////////////////////////
    static void SetHints()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, POE_OPENGL_VERSION_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, POE_OPENGL_VERSION_MINOR);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    }

    ////////////////////////////////////////
    static GLFWwindow* OpenWindow(bool isFullscreen, int width, int height, const char* title)
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

        GLFWwindow* window = glfwCreateWindow(isFullscreen ? mode->width : width,
                                              isFullscreen ? mode->height : height,
                                              title,
                                              isFullscreen ? monitor : nullptr, nullptr);
        if (!window) {
            std::fprintf(stderr, "ERROR: couldn't open window\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
        return window;
    }

    ////////////////////////////////////////
    static void InitOpenGL(GLFWwindow* window)
    {
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            std::fprintf(stderr, "ERROR: couldn't load OpenGL context\n");
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
    }

    ////////////////////////////////////////
    static void DebugOutput()
    {
        DebugUI::PushLog(stdout, "[DEBUG] GL version: %s\n", glGetString(GL_VERSION));
        DebugUI::PushLog(stdout, "[DEBUG] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        DebugUI::PushLog(stdout, "[DEBUG] GL renderer: %s\n", glGetString(GL_RENDERER));
        DebugUI::PushLog(stdout, "[DEBUG] GL vendor: %s\n", glGetString(GL_VENDOR));
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
    GLFWwindow* CreateFullScreenWindow(const char* title)
    {
        InitGLFW();
        SetHints();
        GLFWwindow* window = OpenWindow(true, 1, 1, title);
        InitOpenGL(window);
        DebugOutput();
        EnableDebugContext();
        return window;
    }
}
