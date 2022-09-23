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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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
    unsigned BufferGL::CreateId()
    {
        unsigned id;
        glGenBuffers(1, &id);
        return id;
    }

    ////////////////////////////////////////
    BufferGL::BufferGL(int mode, int numElements)
        : ObjectGL(CreateId()), mMode{mode}, mNumElements{numElements} {}

    ////////////////////////////////////////
    BufferGL::BufferGL(int id, int mode, int numElements)
        : ObjectGL(id), mMode{mode}, mNumElements{numElements} {}

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(const std::vector<float>& vertices, int mode)
        : BufferGL(mode, static_cast<int>(vertices.size()))
    {
        glBindBuffer(GL_ARRAY_BUFFER, mId);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), mode);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        std::printf("[DEBUG] Allocated %ld bytes for vertex buffer %u\n", vertices.size() * sizeof(float), mId);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(VertexBuffer&& other)
        : BufferGL(other.mId, other.mMode, other.mNumElements)
    {
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
        : BufferGL(mode, static_cast<int>(indices.size()))
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), mode);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        std::printf("[DEBUG] Allocated %ld bytes for index buffer %u\n", indices.size() * sizeof(float), mId);
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(IndexBuffer&& other)
        : BufferGL(other.mId, other.mMode, other.mNumElements)
    {
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
    unsigned VAO::CreateId()
    {
        unsigned id;
        glGenVertexArrays(1, &id);
        return id;
    }

    ////////////////////////////////////////
    VAO::VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos)
        : ObjectGL(CreateId()), mNumIndices{ebo.GetNumElements()}
    {
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
        : ObjectGL(other.mId)
    {
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
        : ObjectGL(glCreateShader(type)), mType{type}
    {
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
        : ObjectGL(other.mId)
    {
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
    Program::Program(std::initializer_list<std::reference_wrapper<const Shader>> shaders)
        : ObjectGL(glCreateProgram())
    {
        for (const Shader& shader : shaders)
            glAttachShader(mId, shader.GetId());
        glLinkProgram(mId);

        int success = 0;
        glGetProgramiv(mId, GL_LINK_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetProgramInfoLog(mId, 512, nullptr, infolog);
            std::fprintf(stderr, "ERROR: %s\n", infolog);
        }

        for (const Shader& shader : shaders)
            glDetachShader(mId, shader.GetId());
    }

    ////////////////////////////////////////
    Program::Program(Program&& other)
        : ObjectGL(other.mId)
    {
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
        Shader& vshader = loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/basic.vert");
        Shader& fshader = loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/basic.frag");
        return Program{ vshader, fshader };
    }

    ////////////////////////////////////////
    Program CreateEmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader)
    {
        Shader& vshader = loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/emissive_color.vert");
        Shader& fshader = loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/emissive_color.frag");
        return Program{ vshader, fshader };
    }

    ////////////////////////////////////////
    Program CreateEmissiveTextureProgram(const std::string& rootPath, ShaderLoader& loader)
    {
        Shader& vshader = loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/emissive_texture.vert");
        Shader& fshader = loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/emissive_texture.frag");
        Program program{ vshader, fshader };

        program.Use();
            program.Uniform("uEmissiveTexture", 0);
        program.Halt();
        return program;
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
            -0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,        1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,        1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,        0.0f, 1.0f,

            // right
             0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,        1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,        0.0f, 1.0f,

             // back
             -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,
              0.5f, -0.5f, -0.5f,       1.0f, 0.0f,
              0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
             -0.5f,  0.5f, -0.5f,       0.0f, 1.0f,

             // left
             -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,
             -0.5f, -0.5f,  0.5f,       1.0f, 0.0f,
             -0.5f,  0.5f,  0.5f,       1.0f, 1.0f,
             -0.5f,  0.5f, -0.5f,       0.0f, 1.0f,

             // top
             -0.5f, 0.5f,  0.5f,        0.0f, 0.0f,
              0.5f, 0.5f,  0.5f,        1.0f, 0.0f,
              0.5f, 0.5f, -0.5f,        1.0f, 1.0f,
             -0.5f, 0.5f, -0.5f,        0.0f, 1.0f,

             // bottom
             -0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
              0.5f, -0.5f,  0.5f,       1.0f, 0.0f,
              0.5f, -0.5f, -0.5f,       1.0f, 1.0f,
             -0.5f, -0.5f, -0.5f,       0.0f, 1.0f
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
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
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
    StaticMesh CreatePyramid()
    {
        std::vector<float> vertices{
            0.0f,  0.5f,  0.0f,       0.5f, 1.0f,
           -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,       1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,       1.0f, 0.0f
        };

        std::vector<unsigned> indices{
            0, 2, 1,
            0, 3, 2,
            0, 4, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4
        };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
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
    Shader& ShaderLoader::Load(int type, const std::string& shaderUrl)
    {
        auto iter = mShaders.find(shaderUrl);
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl);
            Shader shader(type, contents);
            auto s = mShaders.insert(std::pair(shaderUrl, std::move(shader)));
            return s.first->second;
        }
        return iter->second;
    }

    ////////////////////////////////////////
    unsigned Texture2D::CreateId()
    {
        unsigned id;
        glGenTextures(1, &id);
        return id;
    }

    ////////////////////////////////////////
    template <typename T>
    void Texture2D::Create(T* data)
    {
        glBindTexture(GL_TEXTURE_2D, mId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mParams.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mParams.wrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mParams.minF);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mParams.magF);
            if (GLAD_GL_EXT_texture_filter_anisotropic) {
                float gpuMaxAnisotropy;
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gpuMaxAnisotropy);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, mParams.maxAnisotropy <= gpuMaxAnisotropy ? mParams.maxAnisotropy : gpuMaxAnisotropy);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, mParams.internalFormat, mWidth, mHeight, 0, mParams.textureFormat, mParams.type, data);
            if (mParams.generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::printf("[DEBUG] Allocated %ld bytes for 2D texture %u\n", mWidth * mHeight * mNumChannels * sizeof(unsigned char) * (mParams.generateMipmaps ? 2 : 1), mId);
    }

    ////////////////////////////////////////
    Texture2D::Texture2D(const std::string& url, const Texture2DParams& params)
        : ObjectGL(CreateId()), mUrl{url}, mParams{params}
    {
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(url.c_str(), &mWidth, &mHeight, &mNumChannels, 0);
        if (!data) {
            std::fprintf(stderr, "[DEBUG] ERROR: couldn't load %s\n", url.c_str());
            return;
        }
        Create(data);
        stbi_image_free(data);
    }

    ////////////////////////////////////////
    template <typename T>
    Texture2D::Texture2D(T* data, int width, int height, int numChannels, const Texture2DParams& params)
        : ObjectGL(CreateId()), mUrl{"<None>"}, mParams{params}
    {
        mWidth = width;
        mHeight = height;
        mNumChannels = numChannels;

        Create(data);
    }

    ////////////////////////////////////////
    Texture2D::Texture2D(Texture2D&& other)
        : ObjectGL(other.mId)
    {
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mNumChannels = other.mNumChannels;
        mUrl = other.mUrl;
        mParams = other.mParams;

        other.mId = 0;
    }

    ////////////////////////////////////////
    Texture2D& Texture2D::operator=(Texture2D&& other)
    {
        if (this != &other) {
            glDeleteTextures(1, &mId);

            mId = other.mId;

            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mNumChannels = other.mNumChannels;
            mUrl = other.mUrl;
            mParams = other.mParams;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Texture2D CreateCheckerboardTexture2D(const glm::vec3& color0, const glm::vec3& color1)
    {
        Texture2DParams params{ .minF = GL_NEAREST,
                                .magF = GL_NEAREST,
                                .type = GL_FLOAT };
        float data[] {
            color0.r, color0.g, color0.b,    color1.r, color1.g, color1.b,
            color1.r, color1.g, color1.b,    color0.r, color0.g, color0.b
        };
        return Texture2D(data, 2, 2, 3, params);
    }

    ////////////////////////////////////////
    Texture2D& Texture2DLoader::Load(const std::string& url, const Texture2DParams& params)
    {
        auto iter = mTextures.find(url);
        if (iter == mTextures.end()) {
            Texture2D texture(url, params);
            auto t = mTextures.insert(std::pair(url, std::move(texture)));
            return t.first->second;
        }
        return iter->second;
    }
}
