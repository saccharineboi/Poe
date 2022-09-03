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
    void APIENTRY GraphicsDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam)
    {
        // ignore non-significant error/warning codes
        if(id == 1 || id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::fprintf(stderr, "\nOpenGL Debug message (%d) : %s\n", id, message);

        switch (source)
        {
            case GL_DEBUG_SOURCE_API:
                std::fprintf(stderr, "Source: API\n");
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                std::fprintf(stderr, "Source: Window System\n");
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                std::fprintf(stderr, "Source: Shader Compiler\n");
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                std::fprintf(stderr, "Source: Third Party\n");
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                std::fprintf(stderr, "Source: Application\n");
                break;
            case GL_DEBUG_SOURCE_OTHER:
                std::fprintf(stderr, "Source: Other\n");
                break;
        };

        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:
                std::fprintf(stderr, "Type: Error\n");
                break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                std::fprintf(stderr, "Type: Deprecated Behaviour\n");
                break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                std::fprintf(stderr, "Type: Undefined Behaviour\n");
                break;
            case GL_DEBUG_TYPE_PORTABILITY:
                std::fprintf(stderr, "Type: Portability\n");
                break;
            case GL_DEBUG_TYPE_PERFORMANCE:
                std::fprintf(stderr, "Type: Performance\n");
                break;
            case GL_DEBUG_TYPE_MARKER:
                std::fprintf(stderr, "Type: Marker\n");
                break;
            case GL_DEBUG_TYPE_PUSH_GROUP:
                std::fprintf(stderr, "Type: Push Group\n");
                break;
            case GL_DEBUG_TYPE_POP_GROUP:
                std::fprintf(stderr, "Type: Pop Group\n");
                break;
            case GL_DEBUG_TYPE_OTHER:
                std::fprintf(stderr, "Type: Other\n");
                break;
        }

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                std::fprintf(stderr, "Severity: high\n");
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                std::fprintf(stderr, "Severity: medium\n");
                break;
            case GL_DEBUG_SEVERITY_LOW:
                std::fprintf(stderr, "Severity: low\n");
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                std::fprintf(stderr, "Severity: notification\n");
                break;
        }
        std::fprintf(stderr, "\n");
    }

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
        auto program = CreateEmissiveColorProgram("../shaders/", shaderLoader);

        program.Use();

        mainCamera.mSensitivity = 0.0025f;
        mainCamera.mPosition = glm::vec3(0.0f, 3.0f, 3.0f);
        mainCamera.mTargetPosition = mainCamera.mPosition;

        float rads = 0.0f;
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float dt = Utility::ComputeDeltaTime();
            mainCamera.Update(dt);

            rads += dt;
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, rads, glm::vec3(1.0f, 0.0f, 1.0f));

            program.Uniform("uPVM", mainCamera.mProjection * mainCamera.mView * model);
            program.Uniform("uModelView", mainCamera.mView * model);
            program.Uniform("uColor", glm::vec4(0.25f, 0.5f, 1.0f, 1.0f));
            program.Uniform("uFogColor", clearColor);
            program.Uniform("uFogDistance", 25.0f);
            program.Uniform("uFogExp", 3.0f);

            staticMesh.Bind();
            staticMesh.Draw();

            program.Uniform("uPVM", mainCamera.mProjection * mainCamera.mView);
            program.Uniform("uModelView", mainCamera.mView);
            program.Uniform("uColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
            grid.Bind();
            grid.Draw(GL_LINES);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return EXIT_SUCCESS;
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(const std::vector<float>& vertices, int mode)
    {
        mMode = mode;
        mNumElements = vertices.size();

        glGenBuffers(1, &mId);
        glBindBuffer(GL_ARRAY_BUFFER, mId);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), mode);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(VertexBuffer&& other)
    {
        mId = other.mId;
        mNumElements = other.mNumElements;
        mMode = other.mMode;

        other.mId = 0;
        other.mNumElements = 0;
    }

    ////////////////////////////////////////
    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
    {
        if (this != &other) {
            glDeleteBuffers(1, &mId);

            mId = other.mId;
            mNumElements = other.mNumElements;
            mMode = other.mMode;

            other.mId = 0;
            other.mNumElements = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(const std::vector<unsigned>& indices, int mode)
    {
        mMode = mode;
        mNumElements = indices.size();

        glGenBuffers(1, &mId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), mode);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(IndexBuffer&& other)
    {
        mId = other.mId;
        mNumElements = other.mNumElements;
        mMode = other.mMode;

        other.mId = 0;
        other.mNumElements = 0;
    }

    ////////////////////////////////////////
    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
    {
        if (this != &other) {
            glDeleteBuffers(1, &mId);

            mId = other.mId;
            mNumElements = other.mNumElements;
            mMode = other.mMode;

            other.mId = 0;
            other.mNumElements = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    VAO::VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos)
        : mNumIndices{static_cast<int>(ebo.GetNumElements())}
    {
        glGenVertexArrays(1, &mId);
        glBindVertexArray(mId);
            vbo.Bind();
            for (const VertexInfo& info : infos) {
                glEnableVertexAttribArray(info.loc);
                glVertexAttribPointer(info.loc, info.numElements, info.dataType, GL_FALSE, info.stride, info.offset);
            }
            vbo.UnBind();
            ebo.Bind();
        glBindVertexArray(0);
        ebo.UnBind();
    }

    ////////////////////////////////////////
    VAO::VAO(VAO&& other)
    {
        mId = other.mId;
        mNumIndices = other.mNumIndices;

        other.mId = 0;
        other.mNumIndices = 0;
    }

    ////////////////////////////////////////
    VAO& VAO::operator=(VAO&& other)
    {
        if (this != &other) {
            glDeleteVertexArrays(1, &mId);

            mId = other.mId;
            mNumIndices = other.mNumIndices;

            other.mId = 0;
            other.mNumIndices = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Shader::Shader(int type, const std::string& source)
        : mType{type}
    {
        mId = glCreateShader(type);
        const char* shaderSrc = source.c_str();
        glShaderSource(mId, 1, &shaderSrc, nullptr);
        glCompileShader(mId);

        int success = 0;
        glGetShaderiv(mId, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetShaderInfoLog(mId, 512, nullptr, infolog);
            std::fprintf(stderr, "ERROR: %s\n", infolog);
        }
    }

    ////////////////////////////////////////
    Shader::Shader(Shader&& other)
    {
        mId = other.mId;
        mType = other.mType;

        other.mId = 0;
    }

    ////////////////////////////////////////
    Shader& Shader::operator=(Shader&& other)
    {
        if (this != &other) {
            glDeleteShader(mId);

            mId = other.mId;
            mType = other.mType;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Program::Program(const std::initializer_list<const Shader*>& shaders)
    {
        mId = glCreateProgram();
        for (const Shader* shader : shaders)
            glAttachShader(mId, shader->GetId());
        glLinkProgram(mId);

        int success = 0;
        glGetProgramiv(mId, GL_LINK_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetProgramInfoLog(mId, 512, nullptr, infolog);
            std::fprintf(stderr, "ERROR: %s\n", infolog);
        }

        for (const Shader* shader : shaders)
            glDetachShader(mId, shader->GetId());
    }

    ////////////////////////////////////////
    Program::Program(Program&& other)
    {
        mId = other.mId;
        other.mId = 0;
    }

    ////////////////////////////////////////
    Program& Program::operator=(Program&& other)
    {
        if (this != &other) {
            glDeleteProgram(mId);
            mId = other.mId;
            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Program CreateBasicProgram(const std::string& rootPath, ShaderLoader& loader)
    {
        Shader vshader(GL_VERTEX_SHADER, loader.Load(rootPath + "basic.vert"));
        Shader fshader(GL_FRAGMENT_SHADER, loader.Load(rootPath + "basic.frag"));
        return Program{ &vshader, &fshader };
    }

    ////////////////////////////////////////
    Program CreateEmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader)
    {
        Shader vshader(GL_VERTEX_SHADER, loader.Load(rootPath + "emissive_color.vert"));
        Shader fshader(GL_FRAGMENT_SHADER, loader.Load(rootPath + "emissive_color.frag"));
        return Program{ &vshader, &fshader };
    }

    ////////////////////////////////////////
    StaticMesh::StaticMesh(const std::vector<float>& vertices,
                           const std::vector<unsigned>& indices,
                           const std::vector<VertexInfo>& infos)
        : mVbo(vertices, GL_STATIC_DRAW),
          mEbo(indices, GL_STATIC_DRAW),
          mVao(mVbo, mEbo, infos)
    {}

    ////////////////////////////////////////
    StaticMesh CreateColoredTriangle()
    {
        std::vector<float> vertices {
            -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
             0.0f,  0.5f,   0.0f, 0.0f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2 };

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(2 * sizeof(float)) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateColoredQuad()
    {
        std::vector<float> vertices {
            -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,   1.0f, 1.0f, 0.0f
        };
        std::vector<unsigned> indices { 0, 1, 2, 2, 1, 3 };

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(2 * sizeof(float)) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateColoredCircle(float radius, int numSegments)
    {
        std::vector<float> vertices;
        vertices.reserve((numSegments + 1) * 5);

        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        vertices.push_back(1.0f);
        vertices.push_back(1.0f);
        vertices.push_back(1.0f);

        const float angleDelta = PI2 / static_cast<float>(numSegments);
        for (float angle = 0.0f; angle < PI2; angle += angleDelta) {
            vertices.push_back(glm::cos(angle) * radius);
            vertices.push_back(glm::sin(angle) * radius);

            vertices.push_back(1.0f);
            vertices.push_back((glm::cos(angle) + 1.0f) * 0.5f);
            vertices.push_back((glm::sin(angle) + 1.0f) * 0.5f);
        }

        std::vector<unsigned> indices;
        indices.reserve((numSegments + 1) * 3);

        for (int i = 0; i < numSegments; ++i) {
            indices.push_back(0);
            indices.push_back(i + 1);
            indices.push_back(i + 2);
        }

        indices.push_back(0);
        indices.push_back(numSegments);
        indices.push_back(1);

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(2 * sizeof(float)) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateTriangle()
    {
        std::vector<float> vertices {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        std::vector<unsigned> indices { 0, 1, 2 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), reinterpret_cast<const void*>(0) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateQuad()
    {
        std::vector<float> vertices {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
             0.5f,  0.5f, 0.0f
        };
        std::vector<unsigned> indices { 0, 1, 2, 2, 1, 3 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), reinterpret_cast<const void*>(0) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateCircle(float radius, int numSegments)
    {
        std::vector<float> vertices;
        vertices.reserve((numSegments + 1) * 3);

        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        const float angleDelta = PI2 / static_cast<float>(numSegments);
        for (float angle = 0.0f; angle < PI2; angle += angleDelta) {
            vertices.push_back(glm::cos(angle) * radius);
            vertices.push_back(glm::sin(angle) * radius);
            vertices.push_back(0.0f);
        }

        std::vector<unsigned> indices;
        indices.reserve((numSegments + 1) * 3);

        for (int i = 0; i < numSegments; ++i) {
            indices.push_back(0);
            indices.push_back(i + 1);
            indices.push_back(i + 2);
        }

        indices.push_back(0);
        indices.push_back(numSegments);
        indices.push_back(1);

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), reinterpret_cast<const void*>(0) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateCube()
    {
        std::vector<float> vertices{
            // front
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            // right
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,

             // back
             -0.5f, -0.5f, -0.5f,
              0.5f, -0.5f, -0.5f,
              0.5f,  0.5f, -0.5f,
             -0.5f,  0.5f, -0.5f,

             // left
             -0.5f, -0.5f, -0.5f,
             -0.5f, -0.5f,  0.5f,
             -0.5f,  0.5f,  0.5f,
             -0.5f,  0.5f, -0.5f,

             // top
             -0.5f, 0.5f,  0.5f,
              0.5f, 0.5f,  0.5f,
              0.5f, 0.5f, -0.5f,
             -0.5f, 0.5f, -0.5f,

             // bottom
             -0.5f, -0.5f,  0.5f,
              0.5f, -0.5f,  0.5f,
              0.5f, -0.5f, -0.5f,
             -0.5f, -0.5f, -0.5f
        };

        std::vector<unsigned> indices{
            // front
            0, 1, 2, 2, 3, 0,
            // right
            4, 5, 6, 6, 7, 4,
            // back
            8, 11, 10, 8, 10, 9,
            // left
            12, 13, 14, 14, 15, 12,
            // top
            16, 17, 18, 18, 19, 16,
            // bottom
            20, 23, 22, 20, 22, 21
        };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), reinterpret_cast<const void*>(0) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateGrid(int numX, int numZ)
    {
        std::vector<float> vertices;
        vertices.reserve(numX * 2 * 3 + numZ * 2 * 3);

        const float startPosX = -0.5f * static_cast<float>(numX - 1);
        constexpr float offsetX = 1.0f;

        const float startPosZ = -0.5f * static_cast<float>(numZ - 1);
        constexpr float offsetZ = 1.0f;

        for (int i = 0; i < numX; ++i) {
            vertices.push_back(startPosX + offsetX * static_cast<float>(i));
            vertices.push_back(0.0f);
            vertices.push_back(startPosZ);

            vertices.push_back(startPosX + offsetX * static_cast<float>(i));
            vertices.push_back(0.0f);
            vertices.push_back(startPosZ + offsetZ * static_cast<float>(numZ - 1));
        }

        for (int i = 0; i < numZ; ++i) {
            vertices.push_back(startPosX);
            vertices.push_back(0.0f);
            vertices.push_back(startPosZ + offsetZ * static_cast<float>(i));

            vertices.push_back(startPosX + offsetX * static_cast<float>(numX - 1));
            vertices.push_back(0.0f);
            vertices.push_back(startPosZ + offsetZ * static_cast<float>(i));
        }

        std::vector<unsigned> indices;
        indices.reserve(2 * (numX + numZ));
        for (int i = 0; i < 2 * (numX + numZ); ++i)
            indices.push_back(i);

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), reinterpret_cast<const void*>(0) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    void FirstPersonCamera::UpdateInputConfig(int key, int action)
    {
        if (key == mInputConfig.moveForwardKey) {
            if (action == GLFW_PRESS) {
                mState.movingForward = true;
                mState.movingBackward = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingForward = false;
        }
        else if (key == mInputConfig.moveBackwardKey) {
            if (action == GLFW_PRESS) {
                mState.movingBackward = true;
                mState.movingForward = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingBackward = false;
        }

        if (key == mInputConfig.moveLeftKey) {
            if (action == GLFW_PRESS) {
                mState.movingLeft = true;
                mState.movingRight = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingLeft = false;
        }
        else if (key == mInputConfig.moveRightKey) {
            if (action == GLFW_PRESS) {
                mState.movingRight = true;
                mState.movingLeft = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingRight = false;
        }

        if (key == mInputConfig.moveUpKey) {
            if (action == GLFW_PRESS) {
                mState.movingUp = true;
                mState.movingDown = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingUp = false;
        }
        else if (key == mInputConfig.moveDownKey) {
            if (action == GLFW_PRESS) {
                mState.movingDown = true;
                mState.movingUp = false;
            }
            else if (action == GLFW_RELEASE)
                mState.movingDown = false;
        }
    }

    ////////////////////////////////////////
    void FirstPersonCamera::Update(float dt)
    {
        if (mState.movingForward)
            mTargetPosition += mDirection * mSpeed * dt;
        else if (mState.movingBackward)
            mTargetPosition -= mDirection * mSpeed * dt;

        if (mState.movingLeft)
            mTargetPosition -= glm::normalize(glm::cross(mDirection, mUp)) * mSpeed * dt;
        else if (mState.movingRight)
            mTargetPosition += glm::normalize(glm::cross(mDirection, mUp)) * mSpeed * dt;

        if (mState.movingUp)
            mTargetPosition += mUp * mSpeed * dt;
        else if (mState.movingDown)
            mTargetPosition -= mUp * mSpeed * dt;

        mPosition = Utility::Lerp(mPosition, mTargetPosition, mSmoothness * dt);

        mView = glm::lookAt(mPosition, mPosition + mDirection, mUp);
        mProjection = glm::perspective(mFovy, mAspectRatio, mNear, mFar);
    }

    ////////////////////////////////////////
    void FirstPersonCamera::UpdateDirection(float mouseX, float mouseY)
    {
#define PITCH_LIMIT (89.0f * D2R)

        static float lastX = mouseX;
        static float lastY = mouseY;

        float dx = mSensitivity * (mouseX - lastX);
        float dy = mSensitivity * (mouseY - lastY);

        lastX = mouseX;
        lastY = mouseY;

        static float pitch;
        static float yaw = -PIH;

        if (mIsMouseCaptured) {
            yaw += dx;
            pitch -= dy;
            if (pitch < -PITCH_LIMIT)
                pitch = -PITCH_LIMIT;
            else if (pitch > PITCH_LIMIT)
                pitch = PITCH_LIMIT;

            mDirection.x = glm::cos(yaw) * glm::cos(pitch);
            mDirection.y = glm::sin(pitch);
            mDirection.z = glm::sin(yaw) * glm::cos(pitch);
        }
    }

    ////////////////////////////////////////
    std::string ShaderLoader::Load(const std::string& shaderUrl)
    {
        auto iter = mShaders.find(shaderUrl);
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl);
            if (contents.length() > 0) {
                mShaders.insert(std::pair(shaderUrl, contents));
                std::printf("[DEBUG] (NEW) (%ld bytes) shader source from %s\n", contents.length(), shaderUrl.c_str());
            }
            return contents;
        }
        std::printf("[DEBUG] (CACHED) (%ld bytes) shader source from %s\n", iter->second.length(), shaderUrl.c_str());
        return iter->second;
    }
}
