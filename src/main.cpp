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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

////////////////////////////////////////
static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);

////////////////////////////////////////
int main()
{
    if (!glfwInit()) {
        std::fprintf(stderr, "ERROR: couldn't initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Poe", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "ERROR: couldn't create window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::fprintf(stderr, "ERROR: couldn't initialize glad\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::printf("[DEBUG] GL version: %s\n", glGetString(GL_VERSION));
    std::printf("[DEBUG] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::printf("[DEBUG] GL renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("[DEBUG] GL vendor: %s\n", glGetString(GL_VENDOR));

#ifdef _DEBUG
    int dflags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &dflags);
    if (dflags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Poe::GraphicsDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::printf("[DEBUG] GL debug output is ON\n");
    }
    else
        std::printf("[DEBUG] GL debug output is OFF\n");
#endif

    glfwSetKeyCallback(window, keyCallback);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    float vertices[] {
        -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.0f,  0.5f,   0.0f, 0.0f, 1.0f
    };
    unsigned indices[] { 0, 1, 2 };

    unsigned vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    unsigned vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    const char* vshaderSrc = "#version 330 core\n"
                             "layout (location = 0) in vec2 aPos;\n"
                             "layout (location = 1) in vec3 aColor;\n"
                             "out vec3 vColor;\n"
                             "void main() {\n"
                             "  gl_Position = vec4(aPos, 0.0f, 1.0f);\n"
                             "  vColor = aColor;\n"
                             "}\n";

    unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vshaderSrc, nullptr);
    glCompileShader(vshader);

    int success = 0;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infolog[512];
        glGetShaderInfoLog(vshader, 512, nullptr, infolog);
        std::fprintf(stderr, "ERROR: %s\n", infolog);
    }

    const char* fshaderSrc = "#version 330 core\n"
                             "in vec3 vColor;\n"
                             "out vec4 color;\n"
                             "void main() {\n"
                             "  color = vec4(vColor, 1.0f);\n"
                             "}";

    unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fshaderSrc, nullptr);
    glCompileShader(fshader);

    success = 0;
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infolog[512];
        glGetShaderInfoLog(fshader, 512, nullptr, infolog);
        std::fprintf(stderr, "ERROR: %s\n", infolog);
    }

    unsigned program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infolog[512];
        glGetProgramInfoLog(program, 512, nullptr, infolog);
        std::fprintf(stderr, "ERROR: %s\n", infolog);
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return EXIT_SUCCESS;
}

////////////////////////////////////////
static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
