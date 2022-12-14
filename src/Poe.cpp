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
#include "Cameras.hpp"

#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/gtc/integer.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Poe
{
    ////////////////////////////////////////
    void APIENTRY GraphicsDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam)
    {
        auto const sourceStr = [source](){
            switch (source)
            {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "WINDOW_SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "SHADER_COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "THIRD_PARTY";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "SOURCE_APPLICATION";
                case GL_DEBUG_SOURCE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }();

        auto const typeStr = [type](){
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR:
                    return "ERROR";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "DEPRECATED_BEHAVIOR";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "UNDEFINED_BEHAVIOR";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "PORTABILITY";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "PERFORMANCE";
                case GL_DEBUG_TYPE_MARKER:
                    return "MARKER";
                case GL_DEBUG_TYPE_PUSH_GROUP:
                    return "PUSH_GROUP";
                case GL_DEBUG_TYPE_POP_GROUP:
                    return "POP_GROUP";
                case GL_DEBUG_TYPE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }();

        auto const severityStr = [severity](){
            switch (severity)
            {
                case GL_DEBUG_SEVERITY_HIGH:
                    return "HIGH";
                case GL_DEBUG_SEVERITY_MEDIUM:
                    return "MEDIUM";
                case GL_DEBUG_SEVERITY_LOW:
                    return "LOW";
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    return "NOTIFICATION";
                default:
                    return "UNKNOWN";
            }
        }();

        DebugUI::PushLog(stderr, "OpenGL Debug [HEADER] (Source: %s) (Type: %s) (Severity: %s)", sourceStr, typeStr, severityStr);
        DebugUI::PushLog(stderr, "OpenGL Debug [DATA] (%d) : %s", id, message);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(const std::vector<float>& vertices, int mode)
        : mMode{mode}, mNumElements{static_cast<int>(vertices.size())}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, vertices.size() * sizeof(float), vertices.data(), mode);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(int numElements, int mode)
        : mMode{mode}, mNumElements{numElements}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, numElements * sizeof(float), nullptr, mode);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(VertexBuffer&& other)
        : mId{other.mId}, mMode{other.mMode}, mNumElements{other.mNumElements}
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
        : mMode{mode}, mNumElements{static_cast<int>(indices.size())}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, indices.size() * sizeof(unsigned), indices.data(), mode);
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(int numElements, int mode)
        : mMode{mode}, mNumElements{numElements}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, numElements * sizeof(unsigned), nullptr, mode);
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(IndexBuffer&& other)
        : mId{other.mId}, mMode{other.mMode}, mNumElements{other.mNumElements}
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
    UniformBuffer::UniformBuffer(int size, int mode, int bindLoc)
        : mSize{size}, mMode{mode}, mBindLoc{bindLoc}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, size, nullptr, mode);
    }

    ////////////////////////////////////////
    UniformBuffer::UniformBuffer(UniformBuffer&& other)
        : mId{other.mId}, mSize{other.mSize}, mMode{other.mMode}, mBindLoc{other.mBindLoc}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
    {
        if (this != &other) {
            glDeleteBuffers(1, &mId);

            mId = other.mId;
            mSize = other.mSize;
            mMode = other.mMode;
            mBindLoc = other.mBindLoc;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    FogUB::FogUB(const glm::vec3& color, float distance, float exponent)
        : mBuffer(sizeof(FogUB__DATA), GL_DYNAMIC_DRAW, 0)
    {
        std::memset(&mData, 0, sizeof(FogUB__DATA));
        mData.SetColor(color);
        mData.distance = distance;
        mData.exponent = exponent;
        mBuffer.Modify(0, sizeof(FogUB__DATA), &mData);
    }

    ////////////////////////////////////////
    TransformUB::TransformUB()
        : mBuffer(sizeof(TransformUB__DATA), GL_DYNAMIC_DRAW, 1)
    {
        std::memset(&mData, 0, sizeof(TransformUB__DATA));
        mBuffer.Modify(0, sizeof(TransformUB__DATA), &mData);
    }

    ////////////////////////////////////////
    void TransformUB::Set(const FirstPersonCamera& camera)
    {
        SetProjectionMatrix(camera.mProjection);
        SetViewMatrix(camera.mView);
        SetProjViewMatrix(camera.mProjection * camera.mView);
        SetCamDir(camera.mDirection);
    }

    ////////////////////////////////////////
    PbrLightMaterialUB::PbrLightMaterialUB()
        : mBuffer(sizeof(PbrLightMaterial__DATA), GL_DYNAMIC_DRAW, 2)
    {
        std::memset(&mData, 0, sizeof(PbrLightMaterial__DATA));
        mBuffer.Modify(0, sizeof(PbrLightMaterial__DATA), &mData);
    }

    ////////////////////////////////////////
    void PbrLightMaterialUB::Set(const PbrLightMaterial& mat)
    {
        SetAlbedo(mat.mAlbedo);
        SetMetallic(mat.mMetallic);
        SetRoughness(mat.mRoughness);
        SetAO(mat.mAo);
    }

    ////////////////////////////////////////
    DirLightUB::DirLightUB()
        : mBuffer(DATA_SIZE, GL_DYNAMIC_DRAW, 3)
    {
        std::memset(&mLightsData, 0, DATA_SIZE);
        mBuffer.Modify(0, DATA_SIZE, mLightsData);
    }

    ////////////////////////////////////////
    void DirLightUB::Set(int ind, const DirLight& dirLight)
    {
        SetColor(ind, dirLight.mColor);
        SetDirection(ind, dirLight.mDirection);
        SetIntensity(ind, dirLight.mIntensity);
    }

    ////////////////////////////////////////
    VAO::VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos)
        : mNumIndices{ebo.GetNumElements()}
    {
        assert(infos.size() > 0);

        const VertexInfo& firstInfo = infos[0];

        glCreateVertexArrays(1, &mId);
        glVertexArrayVertexBuffer(mId, 0, vbo.GetId(), 0, firstInfo.stride);
        glVertexArrayElementBuffer(mId, ebo.GetId());

        for (const VertexInfo& info : infos) {
            glEnableVertexArrayAttrib(mId, info.loc);
            glVertexArrayAttribFormat(mId, info.loc, info.numElements, info.dataType, GL_FALSE, info.offset);
            glVertexArrayAttribBinding(mId, info.loc, 0);
        }
    }

    ////////////////////////////////////////
    VAO::VAO(VAO&& other)
        : mId{other.mId}, mNumIndices{other.mNumIndices}
    {
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
        : mId{glCreateShader(type)}, mType{type}
    {
        assert(source.size() > 0);

        const char* shaderSrc = source.c_str();
        glShaderSource(mId, 1, &shaderSrc, nullptr);
        glCompileShader(mId);

        int success = 0;
        glGetShaderiv(mId, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetShaderInfoLog(mId, 512, nullptr, infolog);
            DebugUI::PushLog(stderr, "[DEBUG] ERROR: %s\n", infolog);
        }
    }

    ////////////////////////////////////////
    Shader::Shader(Shader&& other)
        : mId{other.mId}, mType{other.mType}
    {
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
        : mId{glCreateProgram()}
    {
        for (const Shader& shader : shaders)
            glAttachShader(mId, shader.GetId());
        glLinkProgram(mId);

        int success = 0;
        glGetProgramiv(mId, GL_LINK_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetProgramInfoLog(mId, 512, nullptr, infolog);
            DebugUI::PushLog(stderr, "[DEBUG] ERROR: %s\n", infolog);
        }

        for (const Shader& shader : shaders)
            glDetachShader(mId, shader.GetId());
    }

    ////////////////////////////////////////
    Program::Program(Program&& other)
        : mId{other.mId}, mUniforms{other.mUniforms}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Program& Program::operator=(Program&& other)
    {
        if (this != &other) {
            glDeleteProgram(mId);
            mId = other.mId;
            mUniforms = other.mUniforms;
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
    PostProcessProgram::PostProcessProgram(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/post_process.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/post_process.frag") }
    {
        mProgram.Use();
            mProgram.Uniform("uScreenTexture", 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    void StaticMesh::ReconfigureMatrixBuffer()
    {
        if (mNumInstances > 0) {
            mModelMatrixBuffer->Bind();
            mVao.Bind();
            for (int i = 8; i < 12; ++i) {
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<const void*>((i - 8) * sizeof(glm::vec4)));
                glVertexAttribDivisor(i, 1);
            }
            mVao.UnBind();
            mModelMatrixBuffer->UnBind();
        }
    }

    ////////////////////////////////////////
    void StaticMesh::CreateInstances(std::initializer_list<glm::mat4> modelMatrices)
    {
        int numMatrices = static_cast<int>(modelMatrices.size());
        if (numMatrices > 0) {
            VertexBuffer* oldBuffer = mModelMatrixBuffer.release();
            delete oldBuffer;
            mModelMatrixBuffer.reset(new VertexBuffer(16 * numMatrices, GL_DYNAMIC_DRAW));
            int i{};
            for (const glm::mat4& model : modelMatrices) {
                mModelMatrixBuffer->Modify(i * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));
                ++i;
            }
            mNumInstances = numMatrices;
            ReconfigureMatrixBuffer();
        }
    }

    ////////////////////////////////////////
    void StaticMesh::CreateInstances(const std::vector<glm::mat4>& modelMatrices)
    {
        int numMatrices = static_cast<int>(modelMatrices.size());
        if (numMatrices > 0) {
            VertexBuffer* oldBuffer = mModelMatrixBuffer.release();
            delete oldBuffer;
            mModelMatrixBuffer.reset(new VertexBuffer(16 * numMatrices, GL_DYNAMIC_DRAW));
            float* modelMatrixPtr = mModelMatrixBuffer->GetWritePtr();
            std::memcpy(modelMatrixPtr, modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));
            assert(mModelMatrixBuffer->Unmap() == GL_TRUE);
            mNumInstances = numMatrices;
            ReconfigureMatrixBuffer();
        }
    }

    ////////////////////////////////////////
    void StaticMesh::CreateInstances(int numInstances)
    {
        if (numInstances > 0) {
            VertexBuffer* oldBuffer = mModelMatrixBuffer.release();
            delete oldBuffer;
            mModelMatrixBuffer.reset(new VertexBuffer(16 * numInstances, GL_DYNAMIC_DRAW));
            mNumInstances = numInstances;
            ReconfigureMatrixBuffer();
        }
    }

    ////////////////////////////////////////
    StaticMesh CreateColoredTriangle(int numInstances)
    {
        std::vector<float> vertices {
            -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
             0.0f,  0.5f,   0.0f, 0.0f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2 };

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 0 },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 2 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateColoredQuad(int numInstances)
    {
        std::vector<float> vertices {
            -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,   1.0f, 1.0f, 0.0f
        };
        std::vector<unsigned> indices { 0, 1, 2, 2, 1, 3 };

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 0 },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 2 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateColoredCircle(float radius, int numSegments, int numInstances)
    {
        std::vector<float> vertices;
        vertices.reserve((numSegments + 1) * 5);

        auto add_data = [](auto& buffer, auto... data)
        { (buffer.push_back(data), ...); };

        add_data(vertices, 0.0f, 0.0f);
        add_data(vertices, 1.0f, 1.0f, 1.0f);

        const float angleDelta = PI2 / static_cast<float>(numSegments);
        for (float angle = 0.0f; angle < PI2; angle += angleDelta) {
            add_data(vertices, glm::cos(angle) * radius, glm::sin(angle) * radius);
            add_data(vertices, 1.0f, (glm::cos(angle) + 1.0f) * 0.5f, (glm::sin(angle) + 1.0f) * 0.5f);
        }

        std::vector<unsigned> indices;
        indices.reserve((numSegments + 1) * 3);

        for (int i = 0; i < numSegments; ++i)
            add_data(indices, 0, i + 1, i + 2);
        add_data(indices, 0, numSegments, 1);

        std::vector<VertexInfo> infos{
            { 0, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 0 },
            { 1, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 2 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateTriangle(int numInstances)
    {
        std::vector<float> vertices {
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
             0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       0.5f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateQuad(int numInstances)
    {
        std::vector<float> vertices {
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
            -0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2, 2, 1, 3 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateCircle(float radius, int numSegments, int numInstances)
    {
        std::vector<float> vertices;
        vertices.reserve((numSegments + 1) * 3);

        auto add_data = [](auto& buffer, auto... data)
        { (buffer.push_back(data), ...); };

        add_data(vertices, 0.0f, 0.0f, 0.0f);
        add_data(vertices, 0.0f, 0.0f, 1.0f);
        add_data(vertices, 0.0f, 0.0f);

        const float angleDelta = PI2 / static_cast<float>(numSegments);
        for (float angle = 0.0f; angle < PI2; angle += angleDelta) {
            float xPos = glm::cos(angle) * radius;
            float yPos = glm::sin(angle) * radius;
            float zPos = 0.0f;

            add_data(vertices, xPos, yPos, zPos);
            add_data(vertices, 0.0f, 0.0f, 1.0f);
            add_data(vertices, xPos, yPos);
        }

        std::vector<unsigned> indices;
        indices.reserve((numSegments + 1) * 3);

        for (int i = 0; i < numSegments; ++i)
            add_data(indices, 0, i + 1, i + 2);
        add_data(indices, 0, numSegments, 1);

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateCube(int numInstances)
    {
        std::vector<float> vertices{
            // front
            -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f,

            // right
             0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

             // back
             -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
              0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
              0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
             -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,

             // left
             -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
             -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
             -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
             -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

             // top
             -0.5f, 0.5f,  0.5f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
              0.5f, 0.5f,  0.5f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
              0.5f, 0.5f, -0.5f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
             -0.5f, 0.5f, -0.5f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f,

             // bottom
             -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
              0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
              0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
             -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f
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
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateGrid(int numX, int numZ, int numInstances)
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
            { 0, 3, GL_FLOAT, static_cast<int>(3 * sizeof(float)), 0 }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreatePyramid(int numInstances)
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
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), 3 * sizeof(float) }
        };

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    /// Source: https://songho.ca/opengl/gl_sphere.html
    ////////////////////////////////////////
    StaticMesh CreateUVSphere(int numStacks, int numSectors, int numInstances)
    {
        std::vector<float> vertices;

        const float stackStep = PI / numStacks;
        const float sectorStep = 2.0f * PI / numSectors;

        for (int i = 0; i <= numStacks; ++i) {
            const float stackAngle = PI / 2.0f - i * stackStep;
            const float xy = glm::cos(stackAngle);
            const float z = glm::sin(stackAngle);

            for (int j = 0; j <= numSectors; ++j) {
                const float sectorAngle = j * sectorStep;

                const float x = xy * glm::cos(sectorAngle);
                const float y = xy * glm::sin(sectorAngle);

                // positions
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // normals
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // texcoords
                const float s = static_cast<float>(j) / numSectors;
                const float t = static_cast<float>(i) / numStacks;

                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        std::vector<unsigned> indices;

        for (int i = 0; i < numStacks; ++i) {
            int k1 = i * (numSectors + 1);
            int k2 = k1 + numSectors + 1;

            for (int j = 0; j < numSectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (numStacks - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        DebugUI::PushLog(stdout, "[DEBUG] Created UVSphere (%d vertices, %d indices)\n",
                static_cast<int>(vertices.size()) / 8, static_cast<int>(indices.size()));

        return StaticMesh(numInstances, vertices, indices, infos);
    }

    ////////////////////////////////////////
    /// Source: https://songho.ca/opengl/gl_sphere.html
    ////////////////////////////////////////
    StaticMesh CreateIcoSphere(int numSubdivisions, int numInstances)
    {
        constexpr float S_STEP = 186.0f / 2048.0f;  // horizontal texcoord step
        constexpr float T_STEP = 322.0f / 1024.0f;  // vertical texcoord step

        float icoVertices[12 * 3];

        constexpr float H_ANGLE = PI / 180.0f * 72.0f; // 360 / 5 -> 72 degrees
        constexpr float V_ANGLE = glm::atan(0.5f); // elevation -> 26.565 degrees

        constexpr float z = glm::sin(V_ANGLE);
        constexpr float xy = glm::cos(V_ANGLE);

        float hAngle1 = -PI / 2.0f  - H_ANGLE / 2.0f;
        float hAngle2 = -PI / 2.0f;

        // top vertex (0, 0, 1)
        icoVertices[0] = 0.0f;
        icoVertices[1] = 0.0f;
        icoVertices[2] = 1.0f;

        // 10 vertices at 2nd and 3rd rows
        for (int i = 1; i <= 5; ++i) {
            int i1 = i * 3;         // 2nd row
            int i2 = (i + 5) * 3;   // 3rd row

            icoVertices[i1] = xy * glm::cos(hAngle1);
            icoVertices[i2] = xy * glm::cos(hAngle2);

            icoVertices[i1 + 1] = xy * glm::sin(hAngle1);
            icoVertices[i2 + 1] = xy * glm::sin(hAngle2);

            icoVertices[i1 + 2] = z;
            icoVertices[i2 + 2] = -z;

            hAngle1 += H_ANGLE;
            hAngle2 += H_ANGLE;
        }

        // bottom vertex (0, 0, -1)
        icoVertices[11 * 3] = 0.0f;
        icoVertices[11 * 3 + 1] = 0.0f;
        icoVertices[11 * 3 + 2] = -1.0f;

        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texcoords;
        std::vector<unsigned> indices;
        std::map<std::pair<float, float>, unsigned> sharedIndices; // key -> (s, t)

        auto add_data = [](auto& buffer, auto... data)
        { (buffer.push_back(data), ...); };

        add_data(vertices, icoVertices[0], icoVertices[1], icoVertices[2]);
        add_data(normals, 0.0f, 0.0f, 1.0f);
        add_data(texcoords, S_STEP, 0.0f);

        add_data(vertices, icoVertices[0], icoVertices[1], icoVertices[2]);
        add_data(normals, 0.0f, 0.0f, 1.0f);
        add_data(texcoords, S_STEP * 3, 0.0f);

        add_data(vertices, icoVertices[0], icoVertices[1], icoVertices[2]);
        add_data(normals, 0.0f, 0.0f, 1.0f);
        add_data(texcoords, S_STEP * 5, 0.0f);

        add_data(vertices, icoVertices[0], icoVertices[1], icoVertices[2]);
        add_data(normals, 0.0f, 0.0f, 1.0f);
        add_data(texcoords, S_STEP * 7, 0.0f);

        add_data(vertices, icoVertices[0], icoVertices[1], icoVertices[2]);
        add_data(normals, 0.0f, 0.0f, 1.0f);
        add_data(texcoords, S_STEP * 9, 0.0f);

        add_data(vertices, icoVertices[33], icoVertices[34], icoVertices[35]);
        add_data(normals, 0.0f, 0.0f, -1.0f);
        add_data(texcoords, S_STEP * 2, T_STEP * 3);

        add_data(vertices, icoVertices[33], icoVertices[34], icoVertices[35]);
        add_data(normals, 0.0f, 0.0f, -1.0f);
        add_data(texcoords, S_STEP * 4, T_STEP * 3);

        add_data(vertices, icoVertices[33], icoVertices[34], icoVertices[35]);
        add_data(normals, 0.0f, 0.0f, -1.0f);
        add_data(texcoords, S_STEP * 6, T_STEP * 3);

        add_data(vertices, icoVertices[33], icoVertices[34], icoVertices[35]);
        add_data(normals, 0.0f, 0.0f, -1.0f);
        add_data(texcoords, S_STEP * 8, T_STEP * 3);

        add_data(vertices, icoVertices[33], icoVertices[34], icoVertices[35]);
        add_data(normals, 0.0f, 0.0f, -1.0f);
        add_data(texcoords, S_STEP * 10, T_STEP * 3);

        glm::vec3 v(icoVertices[3], icoVertices[4], icoVertices[5]);
        glm::vec3 n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, 0, T_STEP);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 10, T_STEP);

        v = glm::vec3(icoVertices[18], icoVertices[19], icoVertices[20]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP, T_STEP * 2);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 11, T_STEP * 2);

        v = glm::vec3(icoVertices[6], icoVertices[7], icoVertices[8]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 2, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 2, T_STEP)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[9], icoVertices[10], icoVertices[11]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 4, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 4, T_STEP)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[12], icoVertices[13], icoVertices[14]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 6, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 6, T_STEP)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[15], icoVertices[16], icoVertices[17]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 8, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 8, T_STEP)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[21], icoVertices[22], icoVertices[23]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 3, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 3, T_STEP * 2)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[24], icoVertices[25], icoVertices[26]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 5, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 5, T_STEP * 2)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[27], icoVertices[28], icoVertices[29]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 7, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 7, T_STEP * 2)] = texcoords.size() / 2 - 1;

        v = glm::vec3(icoVertices[30], icoVertices[31], icoVertices[32]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 9, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 9, T_STEP * 2)] = texcoords.size() / 2 - 1;

        add_data(indices, 0, 10, 14);
        add_data(indices, 1, 14, 15);
        add_data(indices, 2, 15, 16);
        add_data(indices, 3, 16, 17);
        add_data(indices, 4, 17, 11);

        add_data(indices, 10, 12, 14);
        add_data(indices, 12, 18, 14);
        add_data(indices, 14, 18, 15);
        add_data(indices, 18, 19, 15);
        add_data(indices, 15, 19, 16);
        add_data(indices, 19, 20, 16);
        add_data(indices, 16, 20, 17);
        add_data(indices, 20, 21, 17);
        add_data(indices, 17, 21, 11);
        add_data(indices, 21, 13, 11);

        add_data(indices, 5, 18, 12);
        add_data(indices, 6, 19, 18);
        add_data(indices, 7, 20, 19);
        add_data(indices, 8, 21, 20);
        add_data(indices, 9, 13, 21);

        auto isOnLineSegment = [&](glm::vec2 a, glm::vec2 b, glm::vec2 c) {
            float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross > EPSILON || cross < -EPSILON)
                return false;
            if ((c.x > a.x && c.x > b.x) || (c.x < a.x && c.x < b.x))
                return false;
            if ((c.y > a.y && c.y > b.y) || (c.y < a.y && c.y < b.y))
                return false;
            return true;
        };

        auto isSharedTexCoord = [&](glm::vec2 t) {
            constexpr float S = 1.0f / 11.0f;
            constexpr float T = 1.0f / 3.0f;

            constexpr glm::vec2 segments[] {
                glm::vec2(S,     0),     glm::vec2(0, T),
                glm::vec2(S,     0),     glm::vec2(S * 2, T),
                glm::vec2(S * 3, 0),     glm::vec2(S * 2, T),
                glm::vec2(S * 3, 0),     glm::vec2(S * 4, T),
                glm::vec2(S * 5, 0),     glm::vec2(S * 4, T),
                glm::vec2(S * 5, 0),     glm::vec2(S * 6, T),
                glm::vec2(S * 7, 0),     glm::vec2(S * 6, T),
                glm::vec2(S * 7, 0),     glm::vec2(S * 8, T),
                glm::vec2(S * 9, 0),     glm::vec2(S * 8, T),
                glm::vec2(S * 9, 0),     glm::vec2(1, T * 2),
                glm::vec2(0,     T),     glm::vec2(S * 2, 1),
                glm::vec2(S * 3, T * 2), glm::vec2(S * 2, 1),
                glm::vec2(S * 3, T * 2), glm::vec2(S * 4, 1),
                glm::vec2(S * 5, T * 2), glm::vec2(S * 4, 1),
                glm::vec2(S * 5, T * 2), glm::vec2(S * 6, 1),
                glm::vec2(S * 7, T * 2), glm::vec2(S * 6, 1),
                glm::vec2(S * 7, T * 2), glm::vec2(S * 8, 1),
                glm::vec2(S * 9, T * 2), glm::vec2(S * 8, 1),
                glm::vec2(S * 9, T * 2), glm::vec2(S * 10, 1),
                glm::vec2(1,     T * 2), glm::vec2(S * 10, 1)
            };

            constexpr int cnt = sizeof(segments) / sizeof(segments[0]);
            for (int i = 0, j = 1; i < cnt; i += 2, j += 2)
                if (isOnLineSegment(segments[i], segments[j], t))
                    return false;
            return true;
        };

        auto computeIndex = [&](auto p, auto n, auto t) {
            if (isSharedTexCoord(t)) {
                auto key = std::make_pair(t.x, t.y);
                auto it = sharedIndices.find(key);
                if (it == sharedIndices.end()) {
                    add_data(vertices, p.x, p.y, p.z);
                    add_data(normals, n.x, n.y, n.z);
                    add_data(texcoords, t.x, t.y);
                    unsigned index = static_cast<unsigned>(texcoords.size() / 2 - 1);
                    sharedIndices[key] = index;
                    return index;
                }
                else
                    return it->second;
            }
            else {
                add_data(vertices, p.x, p.y, p.z);
                add_data(normals, n.x, n.y, n.z);
                add_data(texcoords, t.x, t.y);
                return static_cast<unsigned>(texcoords.size() / 2 - 1);
            }
        };

        for (int i = 1; i <= numSubdivisions; ++i) {
            auto tmpIndices = std::move(indices);
            indices.clear();
            for (int j = 0; j < static_cast<int>(tmpIndices.size()); j += 3) {
                unsigned i1 = tmpIndices[j];
                unsigned i2 = tmpIndices[j + 1];
                unsigned i3 = tmpIndices[j + 2];

                glm::vec3 v1 = glm::vec3(vertices[i1 * 3],
                                         vertices[i1 * 3 + 1],
                                         vertices[i1 * 3 + 2]);

                glm::vec3 v2 = glm::vec3(vertices[i2 * 3],
                                         vertices[i2 * 3 + 1],
                                         vertices[i2 * 3 + 2]);

                glm::vec3 v3 = glm::vec3(vertices[i3 * 3],
                                         vertices[i3 * 3 + 1],
                                         vertices[i3 * 3 + 2]);

                glm::vec2 t1 = glm::vec2(texcoords[i1 * 2],
                                         texcoords[i1 * 2 + 1]);

                glm::vec2 t2 = glm::vec2(texcoords[i2 * 2],
                                         texcoords[i2 * 2 + 1]);

                glm::vec2 t3 = glm::vec2(texcoords[i3 * 2],
                                         texcoords[i3 * 2 + 1]);

                glm::vec3 newV1 = glm::normalize(v1 + v2);
                glm::vec3 newV2 = glm::normalize(v2 + v3);
                glm::vec3 newV3 = glm::normalize(v1 + v3);

                glm::vec2 newT1 = glm::vec2(0.5f) * (t1 + t2);
                glm::vec2 newT2 = glm::vec2(0.5f) * (t2 + t3);
                glm::vec2 newT3 = glm::vec2(0.5f) * (t1 + t3);

                glm::vec3 newN1 = glm::normalize(newV1);
                glm::vec3 newN2 = glm::normalize(newV2);
                glm::vec3 newN3 = glm::normalize(newV3);

                unsigned newI1 = computeIndex(newV1, newN1, newT1);
                unsigned newI2 = computeIndex(newV2, newN2, newT2);
                unsigned newI3 = computeIndex(newV3, newN3, newT3);

                add_data(indices, i1, newI1, newI3);
                add_data(indices, newI1, i2, newI2);
                add_data(indices, newI1, newI2, newI3);
                add_data(indices, newI3, newI2, i3);
            }
        }

        std::vector<float> interleavedData;
        for (int i = 0, j = 0; i < static_cast<int>(vertices.size()); i += 3, j += 2){
            add_data(interleavedData, vertices[i], vertices[i + 1], vertices[i + 2]);
            add_data(interleavedData, normals[i], normals[i + 1], normals[i + 2]);
            add_data(interleavedData, texcoords[j], texcoords[j + 1]);
        }

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        DebugUI::PushLog(stdout, "[DEBUG] Created IcoSphere (%d vertices, %d indices)\n",
                static_cast<int>(vertices.size()) / 3, static_cast<int>(indices.size()));

        return StaticMesh(numInstances, interleavedData, indices, infos);
    }

    ////////////////////////////////////////
    void StaticModel::Load()
    {
        assert(mPath.size() > 0);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(mPath.data(),
                                                 aiProcess_JoinIdenticalVertices |
                                                 aiProcess_Triangulate |
                                                 aiProcess_GenNormals |
                                                 aiProcess_ImproveCacheLocality |
                                                 aiProcess_ValidateDataStructure |
                                                 aiProcess_ImproveCacheLocality |
                                                 aiProcess_RemoveRedundantMaterials |
                                                 aiProcess_FixInfacingNormals |
                                                 aiProcess_FindInvalidData |
                                                 aiProcess_GenUVCoords |
                                                 aiProcess_TransformUVCoords |
                                                 aiProcess_OptimizeMeshes |
                                                 aiProcess_OptimizeGraph |
                                                 aiProcess_FlipUVs);
        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
            DebugUI::PushLog(stderr, "[ERROR] ASSIMP: %s\n", importer.GetErrorString());
            return;
        }
        mDirectory = mPath.substr(0, mPath.find_last_of('/'));
        LoadNode(scene->mRootNode, scene);
#ifdef _DEBUG
        int numVertices{}, numIndices{};
        for (const StaticMesh& mesh : mMeshes) {
            numVertices += mesh.GetNumVertices();
            numIndices += mesh.GetNumIndices();
        }
        int numMeshes = static_cast<int>(mMeshes.size());
        DebugUI::PushLog(stdout, "[DEBUG] Loaded %s (%d vertices, %d indices, %d mesh%s, %d texture%s)\n", mPath.c_str(), numVertices, numIndices, numMeshes, numMeshes > 1 ? "es" : "", mNumTextures, mNumTextures > 1 ? "s" : "");
#else
        int numMeshes = static_cast<int>(mMeshes.size());
        DebugUI::PushLog(stdout, "[DEBUG] Loaded %s (%d mesh%s, %d texture%s)", mPath.c_str(), numMeshes, numMeshes > 1 ? "es" : "", mNumTextures, mNumTextures > 1 ? "s" : "");
#endif
    }

    ////////////////////////////////////////
    void StaticModel::LoadNode(aiNode* node, const aiScene* scene)
    {
        assert(node != nullptr && scene != nullptr);
        for (int i = 0; i < static_cast<int>(node->mNumMeshes); ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            mMeshes.push_back(LoadStaticMesh(mesh, scene));
        }
        for (int i = 0; i < static_cast<int>(node->mNumChildren); ++i)
            LoadNode(node->mChildren[i], scene);
    }

    // ////////////////////////////////////////
    StaticMesh StaticModel::LoadStaticMesh(aiMesh* mesh, const aiScene* scene)
    {
        assert(mesh != nullptr && scene != nullptr);

        static std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 0 },
            { 1, 2, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 6 * sizeof(float) },
            { 2, 3, GL_FLOAT, static_cast<int>(8 * sizeof(float)), 3 * sizeof(float) }
        };

        std::vector<std::reference_wrapper<const Texture2D>> textures;
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material != nullptr)
            textures = Load2DTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

        int numIndices{};
        for (int i = 0; i < static_cast<int>(mesh->mNumFaces); ++i)
            for (int j = 0; j < static_cast<int>(mesh->mFaces[i].mNumIndices); ++j)
                ++numIndices;

        StaticMesh staticMesh(mNumInstances,
                              static_cast<int>(mesh->mNumVertices) * 8,
                              numIndices, infos, textures);

        float* vboPtr = staticMesh.GetVboWritePtr();

        for (int i = 0; i < static_cast<int>(mesh->mNumVertices); ++i) {
            *vboPtr++ = mesh->mVertices[i].x;
            *vboPtr++ = mesh->mVertices[i].y;
            *vboPtr++ = mesh->mVertices[i].z;

            *vboPtr++ = mesh->mNormals[i].x;
            *vboPtr++ = mesh->mNormals[i].y;
            *vboPtr++ = mesh->mNormals[i].z;

            *vboPtr++ = mesh->mTextureCoords[0][i].x;
            *vboPtr++ = mesh->mTextureCoords[0][i].y;
        }
        assert(staticMesh.UnmapVbo() == GL_TRUE);

        unsigned* eboPtr = staticMesh.GetEboWritePtr();
        for (int i = 0; i < static_cast<int>(mesh->mNumFaces); ++i) {
            const aiFace& face = mesh->mFaces[i];
            for (int j = 0; j < static_cast<int>(face.mNumIndices); ++j)
                *eboPtr++ = face.mIndices[j];
        }
        assert(staticMesh.UnmapEbo() == GL_TRUE);
        return staticMesh;
    }

    ///////////////////////////////////////////
    std::vector<std::reference_wrapper<const Texture2D>> StaticModel::Load2DTextures(aiMaterial* material, aiTextureType type, std::string_view typeName)
    {
        std::vector<std::reference_wrapper<const Texture2D>> textures;
        for (int i = 0; i < static_cast<int>(material->GetTextureCount(type)); ++i) {
            aiString str_ai;
            material->GetTexture(type, i, &str_ai);
            std::string str{ str_ai.C_Str() };

            Texture2DParams params{};
            const Texture2D& tex = mTexture2DLoader.Load(mDirectory + '/' + str, params);
            textures.push_back(tex);
            ++mNumTextures;
        }
        return textures;
    }

    ////////////////////////////////////////
    void StaticModel::CreateInstances(std::initializer_list<glm::mat4> modelMatrices)
    {
        std::ranges::for_each(mMeshes, [&](auto& m){ m.CreateInstances(modelMatrices); });
        mNumInstances = static_cast<int>(modelMatrices.size());
    }

    ////////////////////////////////////////
    void StaticModel::CreateInstances(const std::vector<glm::mat4>& modelMatrices)
    {
        std::ranges::for_each(mMeshes, [&](auto& m){ m.CreateInstances(modelMatrices); });
        mNumInstances = static_cast<int>(modelMatrices.size());
    }

    ////////////////////////////////////////
    void StaticModel::CreateInstances(int numInstances)
    {
        std::ranges::for_each(mMeshes, [&](auto& m){ m.CreateInstances(numInstances); });
        mNumInstances = numInstances;
    }

    ////////////////////////////////////////
    StaticModel LoadCsItaly(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/cs_italy/cs_italy.obj", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadDeDust(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/de_dust2/de_dust2.obj", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadBackpack(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/backpack/backpack.obj", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadViceCity(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/GTA1_Vice_City/GTA1 Vice City.dae", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadGTA2Downtown(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/gta2_maps/Downtown.dae", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadGTA2Industrial(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/gta2_maps/Industrial.dae", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadGTA2Residential(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/gta2_maps/Residential.dae", loader);
    }

    ////////////////////////////////////////
    Shader& ShaderLoader::Load(int type, std::string_view shaderUrl)
    {
        auto iter = mShaders.find(shaderUrl.data());
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl.data());
            Shader shader(type, contents);
            auto s = mShaders.insert(std::make_pair(shaderUrl, std::move(shader)));
            return s.first->second;
        }
        return iter->second;
    }

    ////////////////////////////////////////
    template <typename T>
    void Texture2D::Create(T* data)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &mId);

        glTextureParameteri(mId, GL_TEXTURE_WRAP_S, mParams.wrapS);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_T, mParams.wrapT);
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, mParams.minF);
        glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, mParams.magF);

        if (GLAD_GL_EXT_texture_filter_anisotropic) {
            float gpuMaxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gpuMaxAnisotropy);
            glTextureParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, mParams.maxAnisotropy <= gpuMaxAnisotropy ? mParams.maxAnisotropy : gpuMaxAnisotropy);
        }

        if (mParams.generateMipmaps) {
            mNumMipmaps = static_cast<int>(glm::floor(glm::log2(glm::max(mWidth, mHeight)))) + 1;
            glTextureStorage2D(mId, mNumMipmaps, mParams.internalFormat, mWidth, mHeight);
        }
        else {
            mNumMipmaps = 0;
            glTextureStorage2D(mId, 1, mParams.internalFormat, mWidth, mHeight);
        }
        glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, mParams.textureFormat, mParams.type, data);

        if (mParams.generateMipmaps) glGenerateTextureMipmap(mId);
        DebugUI::PushLog(stdout, "[DEBUG] Loaded 2D texture %s (%d:%d:%d, %d mipmaps)\n", mUrl.c_str(), mWidth, mHeight, mNumChannels, mNumMipmaps);
    }

    ////////////////////////////////////////
    Texture2D::Texture2D(const std::string& url, const Texture2DParams& params)
        : mUrl{url}, mParams{params}
    {
        assert(url.size() > 0);

        unsigned char* data = stbi_load(url.c_str(), &mWidth, &mHeight, &mNumChannels, 0);
        if (!data) {
            DebugUI::PushLog(stderr, "[DEBUG] ERROR: couldn't load %s\n", url.c_str());
            return;
        }

        switch (mNumChannels) {
            case 1:
                mParams.textureFormat = GL_RED;
                mParams.internalFormat = GL_R8;
                break;
            case 2:
                mParams.internalFormat = GL_RG8;
                mParams.textureFormat = GL_RG;
                break;
            case 3:
                mParams.textureFormat = GL_RGB;
                mParams.internalFormat = GL_SRGB8;
                break;
            case 4:
                mParams.textureFormat = GL_RGBA;
                mParams.internalFormat = GL_SRGB8_ALPHA8;
                break;
        }

        Create(data);
        stbi_image_free(data);
    }

    ////////////////////////////////////////
    template <typename T>
    Texture2D::Texture2D(T* data, int width, int height, int numChannels, const Texture2DParams& params)
        : mUrl{"<None>"}, mParams{params}
    {
        mWidth = width;
        mHeight = height;
        mNumChannels = numChannels;

        Create(data);
    }

    ////////////////////////////////////////
    Texture2D::Texture2D(Texture2D&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumChannels{other.mNumChannels}, mUrl{other.mUrl}, mParams{other.mParams}, mNumMipmaps{other.mNumMipmaps}
    {
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
            mNumMipmaps = other.mNumMipmaps;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Texture2D CreateCheckerboardTexture2D(const glm::vec3& color0, const glm::vec3& color1)
    {
        Texture2DParams params{};
        params.minF = GL_NEAREST;
        params.magF = GL_NEAREST;
        params.type = GL_FLOAT;

        float data[] {
            color0.r, color0.g, color0.b,    color1.r, color1.g, color1.b,
            color1.r, color1.g, color1.b,    color0.r, color0.g, color0.b
        };
        return Texture2D(data, 2, 2, 3, params);
    }

    ////////////////////////////////////////
    Texture2D CreateFramebufferTexture2D(int width, int height)
    {
        Texture2DParams params{};
        params.minF = params.magF = GL_LINEAR;
        params.wrapS = params.wrapT = GL_CLAMP_TO_EDGE;
        params.generateMipmaps = false;
        params.internalFormat = GL_RGBA16F;
        params.textureFormat = GL_RGBA;
        unsigned char* data = nullptr;
        return Texture2D(data, width, height, 3, params);
    }

    ////////////////////////////////////////
    Texture2D& Texture2DLoader::Load(const std::string& url, const Texture2DParams& params)
    {
        auto iter = mTextures.find(url);
        if (iter == mTextures.end()) {
            Texture2D texture(url, params);
            auto t = mTextures.insert(std::make_pair(url, std::move(texture)));
            return t.first->second;
        }
        return iter->second;
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(std::initializer_list<std::pair<CubemapFace, std::string_view>> faces)
    {
        assert(faces.size() == 6);

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);

        int faceIndex{};
        for (const auto& face : faces) {
            unsigned char* data = stbi_load(face.second.data(), &mWidth, &mHeight, &mNumChannels, 0);
            if (!data) {
                DebugUI::PushLog(stderr, "[DEBUG] ERROR: couldn't load %s\n", face.second.data());
                return;
            }

            if (!faceIndex) {
                mNumMipmaps = static_cast<int>(glm::floor(glm::log2(glm::max(mWidth, mHeight)))) + 1;
                glTextureStorage2D(mId, mNumMipmaps, GL_RGB8, mWidth, mHeight);
            }

            glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            int texType = [](CubemapFace chosenFace){
                switch (chosenFace) {
                    case CubemapFace::Front:
                        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
                    case CubemapFace::Back:
                        return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
                    case CubemapFace::Left:
                        return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
                    case CubemapFace::Right:
                        return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                    case CubemapFace::Top:
                        return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
                    case CubemapFace::Bottom:
                        return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
                }
                return 0;
            }(face.first);
            assert(texType != 0);

            glTextureSubImage3D(mId, 0, 0, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z - texType, mWidth, mHeight, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
            DebugUI::PushLog(stdout, "[DEBUG] Loaded %s (%d:%d:%d, %d mipmaps)\n", face.second.data(), mWidth, mHeight, mNumChannels, mNumMipmaps);
            stbi_image_free(data);
            ++faceIndex;
        }
        glGenerateTextureMipmap(mId);
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(Cubemap&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumChannels{other.mNumChannels}, mNumMipmaps{other.mNumMipmaps}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Cubemap& Cubemap::operator=(Cubemap&& other)
    {
        if (this != &other) {
            glDeleteTextures(1, &mId);

            mId = other.mId;
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mNumChannels = other.mNumChannels;
            mNumMipmaps = other.mNumMipmaps;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Cubemap CreateUlukaiCoronaSkybox(const std::string& root)
    {
        return Cubemap{
            std::make_pair(CubemapFace::Front,  root + "/skyboxes/ulukai/corona_ft.png"),
            std::make_pair(CubemapFace::Back,   root + "/skyboxes/ulukai/corona_bk.png"),
            std::make_pair(CubemapFace::Left,   root + "/skyboxes/ulukai/corona_lf.png"),
            std::make_pair(CubemapFace::Right,  root + "/skyboxes/ulukai/corona_rt.png"),
            std::make_pair(CubemapFace::Top,    root + "/skyboxes/ulukai/corona_up.png"),
            std::make_pair(CubemapFace::Bottom, root + "/skyboxes/ulukai/corona_dn.png")
        };
    }

    ////////////////////////////////////////
    Cubemap CreateUlukaiRedEclipseSkybox(const std::string& root)
    {
        return Cubemap{
            std::make_pair(CubemapFace::Front,  root + "/skyboxes/ulukai/redeclipse_ft.png"),
            std::make_pair(CubemapFace::Back,   root + "/skyboxes/ulukai/redeclipse_bk.png"),
            std::make_pair(CubemapFace::Left,   root + "/skyboxes/ulukai/redeclipse_lf.png"),
            std::make_pair(CubemapFace::Right,  root + "/skyboxes/ulukai/redeclipse_rt.png"),
            std::make_pair(CubemapFace::Top,    root + "/skyboxes/ulukai/redeclipse_up.png"),
            std::make_pair(CubemapFace::Bottom, root + "/skyboxes/ulukai/redeclipse_dn.png")
        };
    }

    ////////////////////////////////////////
    Cubemap CreateCloudySkybox(const std::string& root)
    {
        return Cubemap{
            std::make_pair(CubemapFace::Front,  root + "/skyboxes/clouds1/clouds1_south.bmp"),
            std::make_pair(CubemapFace::Back,   root + "/skyboxes/clouds1/clouds1_north.bmp"),
            std::make_pair(CubemapFace::Left,   root + "/skyboxes/clouds1/clouds1_west.bmp"),
            std::make_pair(CubemapFace::Right,  root + "/skyboxes/clouds1/clouds1_east.bmp"),
            std::make_pair(CubemapFace::Top,    root + "/skyboxes/clouds1/clouds1_up.bmp"),
            std::make_pair(CubemapFace::Bottom, root + "/skyboxes/clouds1/clouds1_down.bmp")
        };
    }

    ////////////////////////////////////////
    Renderbuffer::Renderbuffer(int type, int width, int height)
        : mType{type}, mWidth{width}, mHeight{height}
    {
        glCreateRenderbuffers(1, &mId);
        glNamedRenderbufferStorage(mId, type, width, height);
    }

    ////////////////////////////////////////
    Renderbuffer::Renderbuffer(Renderbuffer&& other)
        : mId{other.mId}, mType{other.mType}, mWidth{other.mWidth}, mHeight{other.mHeight}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other)
    {
        if (this != &other) {
            glDeleteRenderbuffers(1, &mId);

            mId = other.mId;
            mType = other.mType;
            mWidth = other.mWidth;
            mHeight = other.mHeight;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    bool Framebuffer::Check() const
    {
        int status = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            DebugUI::PushLog(stderr, "ERROR: framebuffer %u is not complete", mId);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& colorAttachment)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0, colorAttachment.GetId(), 0);
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& colorAttachment, const Renderbuffer& rbo)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0, colorAttachment.GetId(), 0);
        glNamedFramebufferRenderbuffer(mId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.GetId());
        Check();
    }

    Framebuffer::Framebuffer(const Texture2DMultiSample& colorAttachment, const RenderbufferMultiSample& rbo)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0, colorAttachment.GetId(), 0);
        glNamedFramebufferRenderbuffer(mId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.GetId());
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(Framebuffer&& other)
        : mId{other.mId}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Framebuffer& Framebuffer::operator=(Framebuffer&& other)
    {
        if (this != &other) {
            glDeleteFramebuffers(1, &mId);
            mId = other.mId;
            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Texture2DMultiSample::Texture2DMultiSample(int width, int height, int type, int numSamples)
        : mWidth{width}, mHeight{height}, mType{type}, mNumSamples{numSamples}
    {
        assert(mNumSamples > 0);

        glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &mId);
        glTextureStorage2DMultisample(mId, numSamples, type, width, height, GL_TRUE);
    }

    ////////////////////////////////////////
    Texture2DMultiSample::Texture2DMultiSample(Texture2DMultiSample&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mType{other.mType}, mNumSamples{other.mNumSamples}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Texture2DMultiSample& Texture2DMultiSample::operator=(Texture2DMultiSample&& other)
    {
        if (this != &other) {
            glDeleteTextures(1, &mId);

            mId = other.mId;
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mType = other.mType;
            mNumSamples = other.mNumSamples;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    RenderbufferMultiSample::RenderbufferMultiSample(int type, int width, int height, int numSamples)
        : mType{type}, mWidth{width}, mHeight{height}, mNumSamples{numSamples}
    {
        assert(numSamples > 0);

        glCreateRenderbuffers(1, &mId);
        glNamedRenderbufferStorageMultisample(mId, numSamples, type, width, height);
    }

    ////////////////////////////////////////
    RenderbufferMultiSample::RenderbufferMultiSample(RenderbufferMultiSample&& other)
        : mId{other.mId}, mType{other.mType}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumSamples{other.mNumSamples}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    RenderbufferMultiSample& RenderbufferMultiSample::operator=(RenderbufferMultiSample&& other)
    {
        if (this != &other) {
            glDeleteRenderbuffers(1, &mId);

            mId = other.mId;
            mType = other.mType;
            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mNumSamples = other.mNumSamples;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    PostProcessStack::PostProcessStack(const std::string& shaderRootPath, int width, int height, int numSamples, ShaderLoader& loader)
        : mWidth{width}, mHeight{height}, mNumSamples{numSamples},
          mProgram(shaderRootPath, loader),
          mRboMS(GL_DEPTH24_STENCIL8, mWidth, mHeight, mNumSamples),
          mColor0MS(mWidth, mHeight, GL_RGBA16F, mNumSamples),
          mFboMS(mColor0MS, mRboMS),
          mColor0{CreateFramebufferTexture2D(mWidth, mHeight)},
          mFbo(mColor0) {}

    ////////////////////////////////////////
    AbstractEmissiveColorProgram::AbstractEmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader, const std::string& vshaderUrl)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + vshaderUrl),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/emissive_color.frag") } {}

    ////////////////////////////////////////
    EmissiveColorProgramInstanced::EmissiveColorProgramInstanced(const std::string& rootPath, ShaderLoader& loader) : AbstractEmissiveColorProgram(rootPath, loader, "/shaders/emissive_color_instanced.vert") {}

    ////////////////////////////////////////
    EmissiveColorProgram::EmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader) : AbstractEmissiveColorProgram(rootPath, loader, "/shaders/emissive_color.vert") {}

    ////////////////////////////////////////
    EmissiveTextureProgramInstanced::EmissiveTextureProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/emissive_texture_instanced.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/emissive_texture.frag") }
    {
        mProgram.Use();
            glUniform1i(EMISSIVE_TEXTURE_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    EmissiveTextureProgram::EmissiveTextureProgram(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/emissive_texture.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/emissive_texture.frag") }
    {
        mProgram.Use();
            glUniform1i(EMISSIVE_TEXTURE_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    void TexturedSkyboxProgram::Init()
    {
        mProgram.Use();
        glUniform1i(SKYBOX_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    Cubemap TexturedSkyboxProgram::ChooseCubemap(DefaultSkyboxTexture defaultSkybox, const std::string& rootPath)
    {
        switch (defaultSkybox) {
            case DefaultSkyboxTexture::UlukaiCorona:
                return CreateUlukaiCoronaSkybox(rootPath);
            case DefaultSkyboxTexture::UlukaiRedEclipse:
                return CreateUlukaiRedEclipseSkybox(rootPath);
            case DefaultSkyboxTexture::Cloudy:
                return CreateCloudySkybox(rootPath);
            default:
                return CreateCloudySkybox(rootPath);
        }
    }

    ////////////////////////////////////////
    TexturedSkyboxProgram::TexturedSkyboxProgram(const std::string& rootPath, ShaderLoader& loader, std::initializer_list<std::pair<CubemapFace, std::string_view>> faces)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/texture_skybox.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/texture_skybox.frag") },
         mCubemap{faces}
    {
        Init();
    }

    ////////////////////////////////////////
    TexturedSkyboxProgram::TexturedSkyboxProgram(const std::string& rootPath, ShaderLoader& loader, DefaultSkyboxTexture defaultSkybox)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/texture_skybox.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/texture_skybox.frag") },
         mCubemap{ChooseCubemap(defaultSkybox, rootPath)}
    {
        Init();
    }

    ////////////////////////////////////////
    PbrLightProgramInstanced::PbrLightProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/pbr_light_instanced.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/pbr_light.frag") }
    {}

    ////////////////////////////////////////
    PbrLightProgram::PbrLightProgram(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/pbr_light.vert"),
                    loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/pbr_light.frag") }
    {}
}
