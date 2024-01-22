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

#include "Poe.hpp"
#include "UI.hpp"
#include "IO.hpp"
#include "Utility.hpp"
#include "Cameras.hpp"
#include "Suppress.hpp"
#include "Constants.hpp"

#include <map>

SUPPRESS_WARNINGS()
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/gtc/integer.hpp>
ENABLE_WARNINGS()

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

namespace Poe
{
    ////////////////////////////////////////
    static inline std::string ComputeOpenGLVersionStringForShader()
    {
        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        // assume version >= 330
        return std::string("#version ") + std::to_string(major) + std::to_string(minor) + std::string("0 core\n");
    }

    ////////////////////////////////////////
    static inline std::string ComputeShaderTypeStringForShader(GLuint type)
    {
        switch (type) {
            case GL_VERTEX_SHADER:
                return "#define POE_VERTEX_SHADER\n";
            case GL_FRAGMENT_SHADER:
                return "#define POE_FRAGMENT_SHADER\n";
            case GL_GEOMETRY_SHADER:
                return "#define POE_GEOMETRY_SHADER\n";
            case GL_COMPUTE_SHADER:
                return "#define POE_COMPUTE_SHADER\n";
            default:
                DebugUI::PushLog(stderr, "%u is an unrecognized shader type", type);
                return "error";
        }
    }

    ////////////////////////////////////////
    static inline std::string ComputeDefineStringForShader(const std::string& param)
    {
        return std::string("#define ") + param + '\n';
    }

    ////////////////////////////////////////
    template <typename T>
    static inline std::string ComputeDefineStringForShader(const std::string& param, T value)
    {
        if constexpr (std::is_floating_point<T>()) {
            int integralPart{ static_cast<int>(value) };
            if (Utility::FloatEquals(static_cast<float>(integralPart) - value, 0.0f)) {
                return std::string("#define ") + param + " " + std::to_string(integralPart) + '\n';
            }
        }
        return std::string("#define ") + param + " " + std::to_string(value) + '\n';
    }

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
    int RuntimeStats::NumDrawCalls{};
    int RuntimeStats::NumInstancedDrawCalls{};
    int RuntimeStats::NumTextureBinds{};
    int RuntimeStats::NumVAOBinds{};

    ////////////////////////////////////////
    void RuntimeStats::Reset()
    {
        NumDrawCalls = 0;
        NumInstancedDrawCalls = 0;
        NumTextureBinds = 0;
        NumVAOBinds = 0;
    }

    ////////////////////////////////////////
    GLuint RuntimeStats::CreateQuery(GLenum type)
    {
        GLuint id;
        glGenQueries(1, &id);
        glBeginQuery(type, id);
        return id;
    }

    ////////////////////////////////////////
    int RuntimeStats::GetQueryResult(GLuint query)
    {
        glEndQuery(query);
        int result;
        glGetQueryObjectiv(query, GL_QUERY_RESULT, &result);
        return result;
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(const std::vector<float>& vertices, unsigned mode)
        : mMode{mode}, mNumElements{vertices.size()}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), mode);
    }

    ////////////////////////////////////////
    VertexBuffer::VertexBuffer(size_t numElements, unsigned mode)
        : mMode{mode}, mNumElements{numElements}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, static_cast<GLsizeiptr>(numElements * sizeof(float)), nullptr, mode);
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
    IndexBuffer::IndexBuffer(const std::vector<unsigned>& indices, unsigned mode)
        : mMode{mode}, mNumElements{indices.size()}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned)), indices.data(), mode);
    }

    ////////////////////////////////////////
    IndexBuffer::IndexBuffer(size_t numElements, unsigned mode)
        : mMode{mode}, mNumElements{numElements}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, static_cast<GLsizeiptr>(numElements * sizeof(unsigned)), nullptr, mode);
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
    UniformBuffer::UniformBuffer(size_t size, unsigned mode, unsigned bindLoc)
        : mSize{size}, mMode{mode}, mBindLoc{bindLoc}
    {
        glCreateBuffers(1, &mId);
        glNamedBufferData(mId, static_cast<GLsizeiptr>(size), nullptr, mode);
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
    RealisticSkyboxUB::RealisticSkyboxUB()
        : mBuffer(sizeof(RealisticSkyboxUB__DATA), GL_DYNAMIC_DRAW, UniformBuffer::REALISTIC_SKYBOX_BLOCK_BINDING)
    {
        std::memset(&mData, 0, sizeof(RealisticSkyboxUB__DATA));

        // default earth atmosphere settings
        mData.SetRayleighScatteringCoefficient(glm::vec3(5.5e-6, 13e-6, 22.4e-6));
        mData.SetRayOrigin(glm::vec3(0.0, 6372e3, 0.0));
        mData.SetSunPosition(glm::vec3(0.0f, 1.0f, -1.0f));

        mData.SetSunIntensity(22.0f);
        mData.SetPlanetRadius(6371e3);
        mData.SetAtmosphereRadius(6471e3);
        mData.SetMieScatteringCoefficient(static_cast<float>(21e-6));
        mData.SetRayleighScaleHeight(8e3);
        mData.SetMieScaleHeight(1.2e3);
        mData.SetMiePreferredScatteringDirection(0.758f);

        Update();
    }

    ////////////////////////////////////////
    FogUB::FogUB(const glm::vec3& color, float distance, float exponent)
        : mBuffer(sizeof(FogUB__DATA), GL_DYNAMIC_DRAW, UniformBuffer::FOG_BLOCK_BINDING)
    {
        std::memset(&mData, 0, sizeof(FogUB__DATA));
        mData.SetColor(color);
        mData.distance = distance;
        mData.exponent = exponent;
        mBuffer.Modify(0, sizeof(FogUB__DATA), &mData);
    }

    ////////////////////////////////////////
    TransformUB::TransformUB()
        : mBuffer(sizeof(TransformUB__DATA), GL_DYNAMIC_DRAW, UniformBuffer::TRANSFORM_BLOCK_BINDING)
    {
        std::memset(&mData, 0, sizeof(TransformUB__DATA));
        mBuffer.Modify(0, sizeof(TransformUB__DATA), &mData);
    }

    ////////////////////////////////////////
    void TransformUB::Set(const FirstPersonCamera& camera)
    {
        SetProjectionMatrix(camera.GetProjectionMatrix());
        SetViewMatrix(camera.GetViewMatrix());
        SetProjViewMatrix(camera.GetProjectionMatrix() * camera.GetViewMatrix());
        SetCamDir(camera.mDirection);
    }

    ////////////////////////////////////////
    PbrLightMaterialUB::PbrLightMaterialUB()
        : mBuffer(sizeof(PbrLightMaterial__DATA), GL_DYNAMIC_DRAW, UniformBuffer::PBR_LIGHT_MATERIAL_BLOCK_BINDING)
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
    BlinnPhongMaterialUB::BlinnPhongMaterialUB()
        : mBuffer(sizeof(BlinnPhongMaterial__DATA), GL_DYNAMIC_DRAW, UniformBuffer::BLINN_PHONG_MATERIAL_BLOCK_BINDING)
    {
        std::memset(&mData, 0, sizeof(BlinnPhongMaterial__DATA));
        mBuffer.Modify(0, sizeof(BlinnPhongMaterial__DATA), &mData);
    }

    ////////////////////////////////////////
    PointLightUB::PointLightUB(int numLights)
        : mBuffer(sizeof(PointLightListElem__DATA) * static_cast<size_t>(numLights), GL_DYNAMIC_DRAW, UniformBuffer::POINT_LIGHT_BLOCK_BINDING),
          mLightsData(static_cast<size_t>(numLights)),
          mNumLights{numLights}
    {
        std::memset(mLightsData.data(), 0, sizeof(PointLightListElem__DATA) * static_cast<size_t>(numLights));
        for (int i = 0; i < numLights; ++i)
        {
            mLightsData[static_cast<size_t>(i)].SetConstant(1.0f);
        }
        mBuffer.Modify(0, sizeof(PointLightListElem__DATA) * static_cast<size_t>(numLights), mLightsData.data());
    }
    ////////////////////////////////////////
    SpotLightUB::SpotLightUB(int numLights)
        : mBuffer(sizeof(SpotLightListElem__DATA) * static_cast<size_t>(numLights), GL_DYNAMIC_DRAW, UniformBuffer::SPOT_LIGHT_BLOCK_BINDING),
          mLightsData(static_cast<size_t>(numLights)),
          mNumLights{numLights}
    {
        std::memset(mLightsData.data(), 0, sizeof(SpotLightListElem__DATA) * static_cast<size_t>(numLights));
        for (int i = 0; i < numLights; ++i)
        {
            mLightsData[static_cast<size_t>(i)].SetConstant(1.0f);
        }
        mBuffer.Modify(0, sizeof(SpotLightListElem__DATA) * static_cast<size_t>(numLights), mLightsData.data());
    }

    ////////////////////////////////////////
    PostProcessUB::PostProcessUB()
        : mBuffer(sizeof(PostProcessUB__DATA), GL_DYNAMIC_DRAW, UniformBuffer::POSTPROCESS_BLOCK_BINDING)
    {
        std::memset(&mData, 0, sizeof(PostProcessUB__DATA));
        mData.kernel[4] = 1.0f; // identity kernel
        mBuffer.Modify(0, sizeof(PostProcessUB__DATA), &mData);
    }

    ////////////////////////////////////////
    VAO::VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos)
        : mNumIndices{static_cast<int>(ebo.GetNumElements())}
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
    Shader::Shader(unsigned type, const std::string& source)
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
    PostProcessProgram::PostProcessProgram(const std::string& rootPath, ShaderLoader& loader)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/post_process.glsl"),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/post_process.glsl",
                                { { "POE_USCREEN_TEXTURE_LOC", PostProcessProgram::SCREEN_TEXTURE_LOC },
                                  { "POE_UTEXEL_STRETCH_LOC", PostProcessProgram::TEXELSTRETCH_LOC },
                                  { "POE_POST_PROCESS_BLOCK_LOC", UniformBuffer::POSTPROCESS_BLOCK_BINDING } },
                                { rootPath + "/shaders/post_processing/gamma.glsl" }) }
    {
        mProgram.Use();
            glUniform1i(SCREEN_TEXTURE_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    void StaticMesh::ReconfigureMatrixBuffer()
    {
        if (mNumInstances > 0) {
            mModelMatrixBuffer->Bind();
            mVao.Bind();
            for (unsigned i = INSTANCED_MODEL_LOC; i < INSTANCED_MODEL_LOC + 4; ++i) {
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
            mModelMatrixBuffer.reset(new VertexBuffer(16 * static_cast<size_t>(numMatrices), GL_DYNAMIC_DRAW));
            int i{};
            for (const glm::mat4& model : modelMatrices) {
                mModelMatrixBuffer->Modify(i * static_cast<int>(sizeof(glm::mat4)), sizeof(glm::mat4), glm::value_ptr(model));
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
            mModelMatrixBuffer.reset(new VertexBuffer(16 * static_cast<size_t>(numMatrices), GL_DYNAMIC_DRAW));
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
            mModelMatrixBuffer.reset(new VertexBuffer(16 * static_cast<size_t>(numInstances), GL_DYNAMIC_DRAW));
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
        vertices.reserve((static_cast<size_t>(numSegments) + 1) * 5);

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
        indices.reserve((static_cast<size_t>(numSegments) + 1) * 3);

        for (unsigned i = 0; i < static_cast<unsigned>(numSegments); ++i)
            add_data(indices, 0U, i + 1, i + 2);
        add_data(indices, 0U, static_cast<unsigned>(numSegments), 1U);

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
        vertices.reserve((static_cast<unsigned>(numSegments) + 1) * 3);

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
        indices.reserve((static_cast<unsigned>(numSegments) + 1) * 3);

        for (unsigned i = 0; i < static_cast<unsigned>(numSegments); ++i)
            add_data(indices, 0U, i + 1, i + 2);
        add_data(indices, 0U, static_cast<unsigned>(numSegments), 1U);

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
        vertices.reserve(static_cast<size_t>(numX * 2 * 3 + numZ * 2 * 3));

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
        indices.reserve(static_cast<size_t>(2 * (numX + numZ)));
        for (unsigned i = 0; i < static_cast<unsigned>(2 * (numX + numZ)); ++i)
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

        const float stackStep = PI / static_cast<float>(numStacks);
        const float sectorStep = 2.0f * PI / static_cast<float>(numSectors);

        for (int i = 0; i <= numStacks; ++i) {
            const float stackAngle = PI / 2.0f - static_cast<float>(i) * stackStep;
            const float xy = glm::cos(stackAngle);
            const float z = glm::sin(stackAngle);

            for (int j = 0; j <= numSectors; ++j) {
                const float sectorAngle = static_cast<float>(j) * sectorStep;

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
                const float s = static_cast<float>(j) / static_cast<float>(numSectors);
                const float t = static_cast<float>(i) / static_cast<float>(numStacks);

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
                    indices.push_back(static_cast<unsigned>(k1));
                    indices.push_back(static_cast<unsigned>(k2));
                    indices.push_back(static_cast<unsigned>(k1 + 1));
                }
                if (i != (numStacks - 1)) {
                    indices.push_back(static_cast<unsigned>(k1 + 1));
                    indices.push_back(static_cast<unsigned>(k2));
                    indices.push_back(static_cast<unsigned>(k2 + 1));
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
        add_data(texcoords, 0.0f, T_STEP);

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
        sharedIndices[std::make_pair(S_STEP * 2, T_STEP)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[9], icoVertices[10], icoVertices[11]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 4, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 4, T_STEP)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[12], icoVertices[13], icoVertices[14]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 6, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 6, T_STEP)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[15], icoVertices[16], icoVertices[17]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 8, T_STEP);
        sharedIndices[std::make_pair(S_STEP * 8, T_STEP)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[21], icoVertices[22], icoVertices[23]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 3, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 3, T_STEP * 2)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[24], icoVertices[25], icoVertices[26]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 5, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 5, T_STEP * 2)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[27], icoVertices[28], icoVertices[29]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 7, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 7, T_STEP * 2)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        v = glm::vec3(icoVertices[30], icoVertices[31], icoVertices[32]);
        n = glm::normalize(v);

        add_data(vertices, v.x, v.y, v.z);
        add_data(normals, n.x, n.y, n.z);
        add_data(texcoords, S_STEP * 9, T_STEP * 2);
        sharedIndices[std::make_pair(S_STEP * 9, T_STEP * 2)] = static_cast<unsigned>(texcoords.size()) / 2 - 1;

        add_data(indices, 0U, 10U, 14U);
        add_data(indices, 1U, 14U, 15U);
        add_data(indices, 2U, 15U, 16U);
        add_data(indices, 3U, 16U, 17U);
        add_data(indices, 4U, 17U, 11U);

        add_data(indices, 10U, 12U, 14U);
        add_data(indices, 12U, 18U, 14U);
        add_data(indices, 14U, 18U, 15U);
        add_data(indices, 18U, 19U, 15U);
        add_data(indices, 15U, 19U, 16U);
        add_data(indices, 19U, 20U, 16U);
        add_data(indices, 16U, 20U, 17U);
        add_data(indices, 20U, 21U, 17U);
        add_data(indices, 17U, 21U, 11U);
        add_data(indices, 21U, 13U, 11U);

        add_data(indices, 5U, 18U, 12U);
        add_data(indices, 6U, 19U, 18U);
        add_data(indices, 7U, 20U, 19U);
        add_data(indices, 8U, 21U, 20U);
        add_data(indices, 9U, 13U, 21U);

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

        auto computeIndex = [&](auto p, auto _n, auto t) {
            if (isSharedTexCoord(t)) {
                auto key = std::make_pair(t.x, t.y);
                auto it = sharedIndices.find(key);
                if (it == sharedIndices.end()) {
                    add_data(vertices, p.x, p.y, p.z);
                    add_data(normals, _n.x, _n.y, _n.z);
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
                add_data(normals, _n.x, _n.y, _n.z);
                add_data(texcoords, t.x, t.y);
                return static_cast<unsigned>(texcoords.size() / 2 - 1);
            }
        };

        for (int i = 1; i <= numSubdivisions; ++i) {
            auto tmpIndices = std::move(indices);
            indices.clear();
            for (size_t j = 0; j < tmpIndices.size(); j += 3) {
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
        for (size_t i = 0, j = 0; i < vertices.size(); i += 3, j += 2){
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
                                                 aiProcess_RemoveRedundantMaterials |
                                                 aiProcess_GenUVCoords |
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
        size_t numVertices{}, numIndices{};
        for (const StaticMesh& mesh : mMeshes) {
            numVertices += mesh.GetNumVertices();
            numIndices += mesh.GetNumIndices();
        }
        size_t numMeshes = mMeshes.size();
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

        std::vector<std::reference_wrapper<const Texture2D>> ambientTextures;
        std::vector<std::reference_wrapper<const Texture2D>> diffuseTextures;
        std::vector<std::reference_wrapper<const Texture2D>> specularTextures;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material != nullptr)
        {
            ambientTextures = Load2DTextures(material, aiTextureType_AMBIENT, "texture_ambient");
            diffuseTextures = Load2DTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            specularTextures = Load2DTextures(material, aiTextureType_SPECULAR, "texture_specular");
        }

        size_t numIndices{};
        for (size_t i = 0; i < mesh->mNumFaces; ++i)
            for (int j = 0; j < static_cast<int>(mesh->mFaces[i].mNumIndices); ++j)
                ++numIndices;

        StaticMesh staticMesh(mNumInstances, mesh->mNumVertices * 8, numIndices, infos);
        std::ranges::for_each(ambientTextures, [&](const Texture2D& t){ staticMesh.AddAmbientTexture(t); });
        std::ranges::for_each(diffuseTextures, [&](const Texture2D& t){ staticMesh.AddDiffuseTexture(t); });
        std::ranges::for_each(specularTextures, [&](const Texture2D& t){ staticMesh.AddSpecularTexture(t); });

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
        for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
            aiString str_ai;
            material->GetTexture(type, static_cast<unsigned>(i), &str_ai);
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
    StaticModel LoadSponza(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/Sponza/scene.gltf", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadCsItaly(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/cs_italy/scene.gltf", loader);
    }

    ////////////////////////////////////////
    StaticModel LoadDeDust(const std::string& rootPath, Texture2DLoader& loader)
    {
        return StaticModel(0, rootPath + "/models/de_dust/scene.gltf", loader);
    }

    ////////////////////////////////////////
    Shader& ShaderLoader::Load(GLuint type, const std::string& shaderUrl)
    {
        auto iter = mShaders.find({shaderUrl, type});
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl.data());
            std::string header = ComputeOpenGLVersionStringForShader();
            std::string shaderType = ComputeShaderTypeStringForShader(type);
            Shader shader(type, header + shaderType + contents);
            auto s = mShaders.insert({std::make_pair(shaderUrl, type), std::move(shader)});
            return s.first->second;
        }
        return iter->second;
    }

    ////////////////////////////////////////
    Shader& ShaderLoader::Load(GLuint type, const std::string& shaderUrl, const std::vector<std::pair<std::string, float>>& values)
    {
        std::stringstream valuesStr;
        for (const auto& [ name, value ] : values) {
            valuesStr << std::to_string(value);
        }

        auto iter = mShaders.find({ shaderUrl + valuesStr.str(), type });
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl.data());
            std::string header = ComputeOpenGLVersionStringForShader();
            std::string shaderType = ComputeShaderTypeStringForShader(type);
            std::string valuesAsString = [&values](){
                std::stringstream ss;
                for (const auto& [ name, value ] : values) {
                    ss << ComputeDefineStringForShader(name, value);
                }
                return ss.str();
            }();
            Shader shader(type, header + shaderType + valuesAsString + contents);
            auto s = mShaders.insert({std::make_pair(shaderUrl + valuesStr.str(), type), std::move(shader)});
            return s.first->second;
        }
        return iter->second;
    }

    ////////////////////////////////////////
    Shader& ShaderLoader::Load(GLuint type,
                               const std::string& shaderUrl,
                               const std::vector<std::pair<std::string, float>>& values,
                               const std::vector<std::string>& additionalUrls)
    {
        std::stringstream valuesStr;
        for (const auto& [ name, value ] : values) {
            valuesStr << std::to_string(value);
        }

        auto iter = mShaders.find({ shaderUrl + valuesStr.str(), type });
        if (iter == mShaders.end()) {
            std::string contents = IO::ReadTextFile(shaderUrl.data());
            std::string header = ComputeOpenGLVersionStringForShader();
            std::string shaderType = ComputeShaderTypeStringForShader(type);
            std::string valuesAsString = [&values](){
                std::stringstream ss;
                for (const auto& [ name, value ] : values) {
                    ss << ComputeDefineStringForShader(name, value);
                }
                return ss.str();
            }();
            std::string additionalHeaders = [&additionalUrls](){
                std::stringstream ss;
                for (const std::string& url : additionalUrls) {
                    ss << IO::ReadTextFile(url) << '\n';
                }
                return ss.str();
            }();
            Shader shader(type, header + shaderType + valuesAsString + additionalHeaders + contents);
            auto s = mShaders.insert({std::make_pair(shaderUrl + valuesStr.str(), type), std::move(shader)});
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

        if (mParams.wrapS == GL_CLAMP_TO_BORDER || mParams.wrapT == GL_CLAMP_TO_BORDER) {
            glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(mBorderColor));
        }

        if (!Utility::FloatEquals(mParams.maxAnisotropy, 0.0f) && GLAD_GL_EXT_texture_filter_anisotropic) {
            float gpuMaxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gpuMaxAnisotropy);
            glTextureParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, mParams.maxAnisotropy <= gpuMaxAnisotropy ? mParams.maxAnisotropy : gpuMaxAnisotropy);
        }

        if (mParams.textureFormat == GL_DEPTH_COMPONENT) {
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        }

        if (mParams.generateMipmaps) {
            mNumMipmaps = static_cast<int>(glm::floor(glm::log2(glm::max(mWidth, mHeight)))) + 1;
        }
        else {
            mNumMipmaps = 1;
        }
        glTextureStorage2D(mId, mNumMipmaps, mParams.internalFormat, mWidth, mHeight);
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
                mParams.internalFormat = GL_RGB8;
                break;
            case 4:
                mParams.textureFormat = GL_RGBA;
                mParams.internalFormat = GL_RGBA8;
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
    template <typename T>
    Texture2D::Texture2D(T* data, int width, int height, int numChannels, const Texture2DParams& params, const glm::vec4& borderColor)
        : mUrl{"<None>"}, mParams{params}, mBorderColor{borderColor}
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
    Texture2D CreateDepthMap(int width, int height)
    {
        Texture2DParams params{};
        params.minF = params.magF = GL_LINEAR;
        params.wrapS = params.wrapT = GL_CLAMP_TO_BORDER;
        params.generateMipmaps = false;
        params.type = GL_FLOAT;
        params.textureFormat = GL_DEPTH_COMPONENT;
        params.internalFormat = GL_DEPTH_COMPONENT16;
        params.maxAnisotropy = 0.0f;
        float* data = nullptr;
        return Texture2D(data, width, height, 1, params, glm::vec4(1.0f));
    }

    ////////////////////////////////////////
    Texture2DArray CreateCascadedDepthMap(int width, int height, int numCascades)
    {
        Texture2DArrayParams params{};
        params.minF = params.magF = GL_LINEAR;
        params.wrapS = params.wrapT = GL_CLAMP_TO_BORDER;
        params.generateMipmaps = false;
        params.type = GL_FLOAT;
        params.textureFormat = GL_DEPTH_COMPONENT;
        params.internalFormat = GL_DEPTH_COMPONENT16;
        params.maxAnisotropy = 0.0f;
        std::vector<float*> data(static_cast<size_t>(numCascades));
        return Texture2DArray(data, width, height, 1, params, glm::vec4(1.0f));
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
    Texture2DArray::Texture2DArray(const std::vector<std::string>& urls, const Texture2DArrayParams& params)
        : mDepth{static_cast<int>(urls.size())}, mUrls{urls}, mParams{params}
    {
        std::vector<unsigned char*> textureDataPack;
        for (const std::string& url : mUrls) {
            unsigned char* data = stbi_load(url.c_str(), &mWidth, &mHeight, &mNumChannels, 0);
            if (!data) {
                DebugUI::PushLog(stderr, "[DEBUG] ERROR: couldn't load %s\n", url.c_str());
                return;
            }
            textureDataPack.push_back(data);
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
                mParams.internalFormat = GL_RGB8;
                break;
            case 4:
                mParams.textureFormat = GL_RGBA;
                mParams.internalFormat = GL_RGBA8;
                break;
        }

        Create(textureDataPack);

        for (unsigned char* dataPtr : textureDataPack) {
            stbi_image_free(dataPtr);
        }
    }

    ////////////////////////////////////////
    template <typename T>
    void Texture2DArray::Create(const std::vector<T*>& data)
    {
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mId);

        glTextureParameteri(mId, GL_TEXTURE_WRAP_S, mParams.wrapS);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_T, mParams.wrapT);
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, mParams.minF);
        glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, mParams.magF);

        if (mParams.wrapS == GL_CLAMP_TO_BORDER || mParams.wrapT == GL_CLAMP_TO_BORDER) {
            glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(mBorderColor));
        }

        if (!Utility::FloatEquals(mParams.maxAnisotropy, 0.0f) && GLAD_GL_EXT_texture_filter_anisotropic) {
            float gpuMaxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gpuMaxAnisotropy);
            glTextureParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, mParams.maxAnisotropy <= gpuMaxAnisotropy ? mParams.maxAnisotropy : gpuMaxAnisotropy);
        }

        if (mParams.textureFormat == GL_DEPTH_COMPONENT) {
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        }

        if (mParams.generateMipmaps) {
            mNumMipmaps = static_cast<int>(glm::floor(glm::log2(glm::max(mWidth, mHeight)))) + 1;
        }
        else {
            mNumMipmaps = 1;
        }

        glTextureStorage3D(mId, mNumMipmaps, mParams.internalFormat, mWidth, mHeight, mDepth);
        for (int i = 0; i < static_cast<int>(data.size()); ++i) {
            glTextureSubImage3D(mId, 0, 0, 0, i, mWidth, mHeight, 1, mParams.textureFormat, mParams.type, data[static_cast<size_t>(i)]);
        }

        if (mParams.generateMipmaps) glGenerateTextureMipmap(mId);

        for (int i = 0; i < mDepth; ++i) {
            DebugUI::PushLog(stdout, "[DEBUG] Loaded %dth texture of 2D texture array (%d:%d:%d:%d, %d mipmaps)\n", i, mWidth, mHeight, mDepth, mNumChannels, mNumMipmaps);
        }
    }

    ////////////////////////////////////////
    template <typename T>
    Texture2DArray::Texture2DArray(const std::vector<T*>& data, int width, int height, int numChannels, const Texture2DArrayParams& params)
        : mParams{params}
    {
        for (size_t i = 0; i < data.size(); ++i) {
            mUrls.push_back("<None>");
        }
        mWidth = width;
        mHeight = height;
        mDepth = static_cast<int>(data.size());
        mNumChannels = numChannels;

        Create(data);
    }

    ////////////////////////////////////////
    template <typename T>
    Texture2DArray::Texture2DArray(const std::vector<T*>& data, int width, int height, int numChannels, const Texture2DArrayParams& params, const glm::vec4& borderColor)
        : mParams{params}, mBorderColor{borderColor}
    {
        for (size_t i = 0; i < data.size(); ++i) {
            mUrls.push_back("<None>");
        }
        mWidth = width;
        mHeight = height;
        mDepth = static_cast<int>(data.size());
        mNumChannels = numChannels;

        Create(data);
    }

    ////////////////////////////////////////
    Texture2DArray::Texture2DArray(Texture2DArray&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mDepth{other.mDepth}, mNumChannels{other.mNumChannels}, mUrls{std::move(other.mUrls)}, mParams{other.mParams}, mNumMipmaps{other.mNumMipmaps}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Texture2DArray& Texture2DArray::operator=(Texture2DArray&& other)
    {
        if (this != &other) {
            glDeleteTextures(1, &mId);

            mId = other.mId;

            mWidth = other.mWidth;
            mHeight = other.mHeight;
            mDepth = other.mDepth;
            mNumChannels = other.mNumChannels;
            mUrls = std::move(other.mUrls);
            mParams = other.mParams;
            mNumMipmaps = other.mNumMipmaps;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(std::initializer_list<std::pair<CubemapFace, std::string_view>> faces)
        : mParams{}
    {
        assert(faces.size() == 6);

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);

        glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
    void Cubemap::Create()
    {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);

        if (mParams.generateMipmaps) {
            mNumMipmaps = static_cast<int>(glm::floor(glm::log2(glm::max(mWidth, mHeight)))) + 1;
        }
        else {
            mNumMipmaps = 1;
        }
        glTextureStorage2D(mId, mNumMipmaps, mParams.internalFormat, mWidth, mHeight);

        glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, mParams.magF);
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, mParams.minF);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_S, mParams.wrapS);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_T, mParams.wrapT);
        glTextureParameteri(mId, GL_TEXTURE_WRAP_R, mParams.wrapR);

        if (mParams.wrapS == GL_CLAMP_TO_BORDER ||
            mParams.wrapT == GL_CLAMP_TO_BORDER ||
            mParams.wrapR == GL_CLAMP_TO_BORDER) {
            glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(mBorderColor));
        }

        if (mParams.textureFormat == GL_DEPTH_COMPONENT) {
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        }

        for (int i = 0; i < 6; ++i) {
            glTextureSubImage3D(mId, 0, 0, 0, i, mWidth, mHeight, 1, mParams.textureFormat, mParams.type, nullptr);
        }
        if (mParams.generateMipmaps) {
            glGenerateTextureMipmap(mId);
        }
    }

    ////////////////////////////////////////
    Cubemap CreateDepthCubemap(int width, int height)
    {
        CubemapParams params;
        params.generateMipmaps = false;
        params.internalFormat = GL_DEPTH_COMPONENT16;
        params.textureFormat = GL_DEPTH_COMPONENT;
        params.minF = params.magF = GL_LINEAR;
        params.wrapS = params.wrapT = params.wrapR = GL_CLAMP_TO_EDGE;
        params.type = GL_FLOAT;
        params.maxAnisotropy = 0.0f;
        return Cubemap(width, height, params);
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(int width, int height, const CubemapParams& params)
        : mWidth{width}, mHeight{height}, mNumChannels{}, mNumMipmaps{}, mParams{params}
    {
        Create();
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(int width, int height, const CubemapParams& params, const glm::vec4& borderColor)
        : mWidth{width}, mHeight{height}, mNumChannels{}, mNumMipmaps{}, mParams{params}, mBorderColor{borderColor}
    {
        Create();
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(Cubemap&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumChannels{other.mNumChannels}, mNumMipmaps{other.mNumMipmaps}, mParams{other.mParams}
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
            mParams = other.mParams;

            other.mId = 0;
        }
        return *this;
    }

    ////////////////////////////////////////
    Renderbuffer::Renderbuffer(unsigned type, int width, int height)
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
        unsigned status = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            DebugUI::PushLog(stderr, "ERROR: framebuffer %u is not complete", mId);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& attachment)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0, attachment.GetId(), 0);
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& attachment, unsigned attachmentType)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, attachmentType, attachment.GetId(), 0);
        if (attachmentType == GL_DEPTH_ATTACHMENT && attachment.GetTextureFormat() == GL_DEPTH_COMPONENT) {
            glNamedFramebufferDrawBuffer(mId, GL_NONE);
            glNamedFramebufferReadBuffer(mId, GL_NONE);
        }
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2DArray& attachment, unsigned attachmentType, int layer)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTextureLayer(mId, attachmentType, attachment.GetId(), 0, layer);
        if (attachmentType == GL_DEPTH_ATTACHMENT && attachment.GetTextureFormat() == GL_DEPTH_COMPONENT) {
            glNamedFramebufferDrawBuffer(mId, GL_NONE);
            glNamedFramebufferReadBuffer(mId, GL_NONE);
        }
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Cubemap& attachment, unsigned attachmentType)
    {
        glCreateFramebuffers(1, &mId);
        glNamedFramebufferTexture(mId, attachmentType, attachment.GetId(), 0);
        if (attachmentType == GL_DEPTH_ATTACHMENT && attachment.GetTextureFormat() == GL_DEPTH_COMPONENT) {
            glNamedFramebufferDrawBuffer(mId, GL_NONE);
            glNamedFramebufferReadBuffer(mId, GL_NONE);
        }
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
    Texture2DMultiSample::Texture2DMultiSample(int width, int height, unsigned type, int numSamples)
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
    RenderbufferMultiSample::RenderbufferMultiSample(unsigned type, int width, int height, int numSamples)
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
    PostProcessStack::PostProcessStack(const std::string& shaderRootPath,
                                       int width, int height,
                                       int numSamples, ShaderLoader& loader)
        : mWidth{width}, mHeight{height}, mOutputWidth{width}, mOutputHeight{height}, mNumSamples{numSamples},
          mProgram(shaderRootPath, loader),
          mRboMS(GL_DEPTH24_STENCIL8, mWidth, mHeight, mNumSamples),
          mColor0MS(mWidth, mHeight, GL_RGBA16F, mNumSamples),
          mFboMS(mColor0MS, mRboMS),
          mColor0{CreateFramebufferTexture2D(mOutputWidth, mOutputHeight)},
          mRbo(GL_DEPTH24_STENCIL8, mWidth, mHeight),
          mFbo(mColor0)
    {
        assert(numSamples > 1);

        mBlock.SetExposure(PP_DEFAULT_EXPOSURE);
        mBlock.SetGamma(PP_DEFAULT_GAMMA);
        mBlock.Buffer().TurnOn();
    }

    ////////////////////////////////////////
    PostProcessStack::PostProcessStack(const std::string& shaderRootPath,
                                       int width, int height,
                                       int outputWidth, int outputHeight,
                                       int numSamples, ShaderLoader& loader)
        : mWidth{width}, mHeight{height}, mOutputWidth{outputWidth}, mOutputHeight{outputHeight}, mNumSamples{numSamples},
          mProgram(shaderRootPath, loader),
          mRboMS(GL_DEPTH24_STENCIL8, mWidth, mHeight, mNumSamples),
          mColor0MS(mWidth, mHeight, GL_RGBA16F, mNumSamples),
          mFboMS(mColor0MS, mRboMS),
          mColor0{CreateFramebufferTexture2D(mOutputWidth, mOutputHeight)},
          mRbo(GL_DEPTH24_STENCIL8, mWidth, mHeight),
          mFbo(mColor0)
    {
        assert(numSamples > 1);

        mBlock.SetExposure(PP_DEFAULT_EXPOSURE);
        mBlock.SetGamma(PP_DEFAULT_GAMMA);
        mBlock.Buffer().TurnOn();
    }

    ////////////////////////////////////////
    PostProcessStack::PostProcessStack(const std::string& shaderRootPath,
                                       int width, int height,
                                       ShaderLoader& loader)
        : mWidth{width}, mHeight{height}, mOutputWidth{width}, mOutputHeight{height}, mNumSamples{1},
          mProgram(shaderRootPath, loader),
          mRboMS(GL_DEPTH24_STENCIL8, mWidth, mHeight, mNumSamples),
          mColor0MS(mWidth, mHeight, GL_RGBA16F, mNumSamples),
          mFboMS(mColor0MS, mRboMS),
          mColor0{CreateFramebufferTexture2D(mOutputWidth, mOutputHeight)},
          mRbo(GL_DEPTH24_STENCIL8, mWidth, mHeight),
          mFbo(mColor0, mRbo)
    {
        mBlock.SetExposure(PP_DEFAULT_EXPOSURE);
        mBlock.SetGamma(PP_DEFAULT_GAMMA);
        mBlock.Buffer().TurnOn();
    }

    ////////////////////////////////////////
    AbstractEmissiveColorProgram::AbstractEmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader, bool isInstanced)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/emissive_color.glsl",
                                { { "POE_APOS_LOC", ATTRIB_POS_LOC },
                                  { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING },
                                  { "POE_UMODEL_LOC", AbstractEmissiveColorProgram::MODEL_LOC },
                                  { "POE_AMODEL_LOC", INSTANCED_MODEL_LOC },
                                  { "POE_INSTANCED", isInstanced ? 1 : 0 } }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/emissive_color.glsl",
                                { { "POE_UCOLOR_LOC", AbstractEmissiveColorProgram::COLOR_LOC },
                                  { "POE_FOG_BLOCK_LOC", UniformBuffer::FOG_BLOCK_BINDING } },
                                { rootPath + "/shaders/post_processing/fog.glsl" }) } {}

    ////////////////////////////////////////
    EmissiveColorProgramInstanced::EmissiveColorProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : AbstractEmissiveColorProgram(rootPath, loader, true) {}

    ////////////////////////////////////////
    EmissiveColorProgram::EmissiveColorProgram(const std::string& rootPath, ShaderLoader& loader)
        : AbstractEmissiveColorProgram(rootPath, loader, false) {}

    ////////////////////////////////////////
    AbstractEmissiveTextureProgram::AbstractEmissiveTextureProgram(const std::string& rootPath, ShaderLoader& loader, bool isInstanced)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/emissive_texture.glsl",
                                { { "POE_APOS_LOC", ATTRIB_POS_LOC },
                                  { "POE_ATEXCOORD_LOC", ATTRIB_TEXCOORD_LOC },
                                  { "POE_UMODEL_LOC", AbstractEmissiveTextureProgram::MODEL_LOC },
                                  { "POE_AMODEL_LOC", INSTANCED_MODEL_LOC },
                                  { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING },
                                  { "POE_INSTANCED", isInstanced ? 1 : 0 } }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/emissive_texture.glsl",
                                { { "POE_UEMISSIVE_TEXTURE_LOC", EMISSIVE_TEXTURE_LOC },
                                  { "POE_UTILE_MULTIPLIER_LOC", TILE_MULTIPLIER_LOC },
                                  { "POE_UTILE_OFFSET_LOC", TILE_OFFSET_LOC },
                                  { "POE_POST_PROCESS_BLOCK_LOC", UniformBuffer::POSTPROCESS_BLOCK_BINDING },
                                  { "POE_FOG_BLOCK_LOC", UniformBuffer::FOG_BLOCK_BINDING } },
                                { rootPath + "/shaders/post_processing/fog.glsl",
                                  rootPath + "/shaders/post_processing/gamma.glsl" }) }
    {
        mProgram.Use();
            glUniform1i(EMISSIVE_TEXTURE_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    EmissiveTextureProgramInstanced::EmissiveTextureProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : AbstractEmissiveTextureProgram(rootPath, loader, true) {}

    ////////////////////////////////////////
    EmissiveTextureProgram::EmissiveTextureProgram(const std::string& rootPath, ShaderLoader& loader)
        : AbstractEmissiveTextureProgram(rootPath, loader, false) {}

    ////////////////////////////////////////
    void TexturedSkyboxProgram::Init()
    {
        mProgram.Use();
        glUniform1i(SKYBOX_LOC, 0);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    Cubemap TexturedSkyboxProgram::ChooseCubemap(DefaultSkyboxTexture defaultSkybox, const std::string& root)
    {
        switch (defaultSkybox) {
            case DefaultSkyboxTexture::Techno:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Techno/vz_techno_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Techno/vz_techno_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Techno/vz_techno_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Techno/vz_techno_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Techno/vz_techno_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Techno/vz_techno_down.png")
                };
            case DefaultSkyboxTexture::Sunshine:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Sunshine/vz_sunshine_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Sunshine/vz_sunshine_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Sunshine/vz_sunshine_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Sunshine/vz_sunshine_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Sunshine/vz_sunshine_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Sunshine/vz_sunshine_down.png")
                };
            case DefaultSkyboxTexture::SinisterOcean:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Sinister Ocean/vz_sinister_ocean_down.png")
                };
            case DefaultSkyboxTexture::SinisterLand:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Sinister Land/vz_sinister_land_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Sinister Land/vz_sinister_land_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Sinister Land/vz_sinister_land_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Sinister Land/vz_sinister_land_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Sinister Land/vz_sinister_land_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Sinister Land/vz_sinister_land_down.png")
                };
            case DefaultSkyboxTexture::Sinister:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Sinister/vz_sinister_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Sinister/vz_sinister_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Sinister/vz_sinister_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Sinister/vz_sinister_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Sinister/vz_sinister_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Sinister/vz_sinister_down.png")
                };
            case DefaultSkyboxTexture::Netherworld:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Netherworld/vz_netherworld_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Netherworld/vz_netherworld_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Netherworld/vz_netherworld_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Netherworld/vz_netherworld_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Netherworld/vz_netherworld_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Netherworld/vz_netherworld_down.png")
                };
            case DefaultSkyboxTexture::Moody:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Moody/vz_moody_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Moody/vz_moody_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Moody/vz_moody_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Moody/vz_moody_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Moody/vz_moody_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Moody/vz_moody_down.png")
                };
            case DefaultSkyboxTexture::Gray:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Gray/vz_gray_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Gray/vz_gray_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Gray/vz_gray_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Gray/vz_gray_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Gray/vz_gray_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Gray/vz_gray_down.png")
                };
            case DefaultSkyboxTexture::EmptySpace:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Empty Space/vz_empty_space_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Empty Space/vz_empty_space_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Empty Space/vz_empty_space_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Empty Space/vz_empty_space_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Empty Space/vz_empty_space_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Empty Space/vz_empty_space_down.png")
                };
            case DefaultSkyboxTexture::DuskOcean:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Dusk Ocean/vz_dusk_ocean_down.png")
                };
            case DefaultSkyboxTexture::DuskLand:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Dusk Land/vz_dusk_land_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Dusk Land/vz_dusk_land_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Dusk Land/vz_dusk_land_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Dusk Land/vz_dusk_land_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Dusk Land/vz_dusk_land_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Dusk Land/vz_dusk_land_down.png")
                };
            case DefaultSkyboxTexture::Dusk:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Dusk/vz_dusk_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Dusk/vz_dusk_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Dusk/vz_dusk_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Dusk/vz_dusk_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Dusk/vz_dusk_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Dusk/vz_dusk_down.png")
                };
            case DefaultSkyboxTexture::Dawn:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Dawn/vz_dawn_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Dawn/vz_dawn_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Dawn/vz_dawn_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Dawn/vz_dawn_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Dawn/vz_dawn_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Dawn/vz_dawn_down.png")
                };
            case DefaultSkyboxTexture::ClearOcean:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Clear Ocean/vz_clear_ocean_down.png")
                };
            case DefaultSkyboxTexture::Clear:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Clear/vz_clear_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Clear/vz_clear_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Clear/vz_clear_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Clear/vz_clear_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Clear/vz_clear_down.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Clear/vz_clear_up.png")
                };
            case DefaultSkyboxTexture::ClassicLand:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Classic Land/vz_classic_land_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Classic Land/vz_classic_land_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Classic Land/vz_classic_land_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Classic Land/vz_classic_land_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Classic Land/vz_classic_land_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Classic Land/vz_classic_land_down.png")
                };
            case DefaultSkyboxTexture::Classic:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Classic/vz_classic_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Classic/vz_classic_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Classic/vz_classic_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Classic/vz_classic_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Classic/vz_classic_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Classic/vz_classic_down.png")
                };
            case DefaultSkyboxTexture::ApocalypseOcean:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Apocalypse Ocean/vz_apocalypse_ocean_down.png")
                };
            case DefaultSkyboxTexture::ApocalypseLand:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Apocalypse Land/vz_apocalypse_land_down.png")
                };
            case DefaultSkyboxTexture::Apocalypse:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/retro/Apocalypse/vz_apocalypse_front.png"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/retro/Apocalypse/vz_apocalypse_back.png"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/retro/Apocalypse/vz_apocalypse_left.png"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/retro/Apocalypse/vz_apocalypse_right.png"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/retro/Apocalypse/vz_apocalypse_up.png"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/retro/Apocalypse/vz_apocalypse_down.png")
                };
            case DefaultSkyboxTexture::Cloudy:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/clouds1/clouds1_north.bmp"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/clouds1/clouds1_south.bmp"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/clouds1/clouds1_west.bmp"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/clouds1/clouds1_east.bmp"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/clouds1/clouds1_down.bmp"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/clouds1/clouds1_up.bmp")
                };
            default:
                return Cubemap{
                    std::make_pair(CubemapFace::Front,  root + "/skyboxes/clouds1/clouds1_north.bmp"),
                    std::make_pair(CubemapFace::Back,   root + "/skyboxes/clouds1/clouds1_south.bmp"),
                    std::make_pair(CubemapFace::Left,   root + "/skyboxes/clouds1/clouds1_west.bmp"),
                    std::make_pair(CubemapFace::Right,  root + "/skyboxes/clouds1/clouds1_east.bmp"),
                    std::make_pair(CubemapFace::Top,    root + "/skyboxes/clouds1/clouds1_down.bmp"),
                    std::make_pair(CubemapFace::Bottom, root + "/skyboxes/clouds1/clouds1_up.bmp")
                };
        }
    }

    ////////////////////////////////////////
    TexturedSkyboxProgram::TexturedSkyboxProgram(const std::string& rootPath, ShaderLoader& loader, std::initializer_list<std::pair<CubemapFace, std::string_view>> faces)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/texture_skybox.glsl",
                                { { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING } }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/texture_skybox.glsl",
                                { { "POE_USKYBOX_LOC", SKYBOX_LOC },
                                  { "POE_POST_PROCESS_BLOCK_LOC", UniformBuffer::POSTPROCESS_BLOCK_BINDING } },
                                { rootPath + "/shaders/post_processing/gamma.glsl" }) },
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

    ////////////////////////////////////////
    AbstractBlinnPhongProgram::AbstractBlinnPhongProgram(const std::string& rootPath,
                                                         ShaderLoader& loader,
                                                         bool isInstanced,
                                                         int numDirLights,
                                                         int numPointLights,
                                                         int numSpotLights,
                                                         int numCascades,
                                                         float shadowBiasMin,
                                                         float shadowBiasMax,
                                                         float pointShadowBias)
        :  mNumDirLights{numDirLights}, mNumPointLights{numPointLights}, mNumSpotLights{numSpotLights},
           mNumCascades{numCascades}, mShadowBiasMin{shadowBiasMin}, mShadowBiasMax{shadowBiasMax},
           mPointShadowBias{pointShadowBias},
           mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/blinn_phong.glsl",
                                { { "NUM_DIR_LIGHTS", numDirLights },
                                  { "NUM_POINT_LIGHTS", numPointLights },
                                  { "NUM_SPOT_LIGHTS", numSpotLights },
                                  { "NUM_CASCADES", numCascades },
                                  { "POE_APOS_LOC", ATTRIB_POS_LOC },
                                  { "POE_ATEXCOORD_LOC", ATTRIB_TEXCOORD_LOC },
                                  { "POE_ANORM_LOC", ATTRIB_NORMAL_LOC },
                                  { "POE_AMODEL_LOC", INSTANCED_MODEL_LOC },
                                  { "POE_ANORM_MAT_LOC", INSTANCED_NORMAL_LOC },
                                  { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING },
                                  { "POE_UMODEL_LOC", MODEL_MATRIX_LOC },
                                  { "POE_UNORM_LOC", NORMAL_MATRIX_LOC },
                                  { "POE_UTEX_OFFSET_LOC", TEX_OFFSET_LOC },
                                  { "POE_UTEX_MULTIPLIER_LOC", TEX_MULTIPLIER_LOC },
                                  { "POE_DIR_LIGHT_BLOCK_LOC", UniformBuffer::DIR_LIGHT_BLOCK_BINDING },
                                  { "POE_POINT_LIGHT_BLOCK_LOC", UniformBuffer::POINT_LIGHT_BLOCK_BINDING },
                                  { "POE_SPOT_LIGHT_BLOCK_LOC", UniformBuffer::SPOT_LIGHT_BLOCK_BINDING },
                                  { "POE_INSTANCED", isInstanced ? 1 : 0 } },
                                { rootPath + "/shaders/lights/directional.glsl",
                                  rootPath + "/shaders/lights/point.glsl",
                                  rootPath + "/shaders/lights/spot.glsl" }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/blinn_phong.glsl",
                                { { "NUM_DIR_LIGHTS", numDirLights },
                                  { "NUM_POINT_LIGHTS", numPointLights },
                                  { "NUM_SPOT_LIGHTS", numSpotLights },
                                  { "NUM_CASCADES", numCascades },
                                  { "SHADOW_BIAS_MIN", shadowBiasMin },
                                  { "SHADOW_BIAS_MAX", shadowBiasMax },
                                  { "POINT_SHADOW_BIAS", pointShadowBias },
                                  { "NUM_CASCADES", numCascades },
                                  { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING },
                                  { "POE_FOG_BLOCK_LOC", UniformBuffer::FOG_BLOCK_BINDING },
                                  { "POE_BLINN_PHONG_MATERIAL_BLOCK_LOC", UniformBuffer::BLINN_PHONG_MATERIAL_BLOCK_BINDING },
                                  { "POE_DIR_LIGHT_BLOCK_LOC", UniformBuffer::DIR_LIGHT_BLOCK_BINDING },
                                  { "POE_POINT_LIGHT_BLOCK_LOC", UniformBuffer::POINT_LIGHT_BLOCK_BINDING },
                                  { "POE_SPOT_LIGHT_BLOCK_LOC", UniformBuffer::SPOT_LIGHT_BLOCK_BINDING },
                                  { "POE_POST_PROCESS_BLOCK_LOC", UniformBuffer::POSTPROCESS_BLOCK_BINDING },
                                  { "POE_UMATERIAL_AMBIENT_TEXTURE_LOC", MATERIAL_AMBIENT_TEXTURE_LOC },
                                  { "POE_UMATERIAL_DIFFUSE_TEXTURE_LOC", MATERIAL_DIFFUSE_TEXTURE_LOC },
                                  { "POE_UMATERIAL_SPECULAR_TEXTURE_LOC", MATERIAL_SPECULAR_TEXTURE_LOC },
                                  { "POE_UAMBIENT_FACTOR_LOC", AMBIENT_FACTOR_LOC },
                                  { "POE_UDIR_LIGHT_DEPTH_MAP_LOC", DIR_LIGHT_DEPTH_MAP },
                                  { "POE_UPOINT_LIGHT_DEPTH_MAP_LOC", POINT_LIGHT_DEPTH_MAP },
                                  { "POE_USPOT_LIGHT_DEPTH_MAP_LOC", SPOT_LIGHT_DEPTH_MAP } },
                                { rootPath + "/shaders/lights/directional.glsl",
                                  rootPath + "/shaders/lights/point.glsl",
                                  rootPath + "/shaders/lights/spot.glsl",
                                  rootPath + "/shaders/post_processing/gamma.glsl",
                                  rootPath + "/shaders/post_processing/fog.glsl",
                                  rootPath + "/shaders/shadows/directional.glsl",
                                  rootPath + "/shaders/shadows/point.glsl",
                                  rootPath + "/shaders/shadows/spot.glsl" }) }
    {
        mProgram.Use();
            glUniform1i(MATERIAL_AMBIENT_TEXTURE_LOC, 0);
            glUniform1i(MATERIAL_DIFFUSE_TEXTURE_LOC, 1);
            glUniform1i(MATERIAL_SPECULAR_TEXTURE_LOC, 2);

            glUniform1i(DIR_LIGHT_DEPTH_MAP, DIR_LIGHT_DEPTH_MAP_BIND_POINT);
            glUniform1i(POINT_LIGHT_DEPTH_MAP, POINT_LIGHT_DEPTH_MAP_BIND_POINT);
            glUniform1i(SPOT_LIGHT_DEPTH_MAP, SPOT_LIGHT_DEPTH_MAP_BIND_POINT);
        mProgram.Halt();
    }

    ////////////////////////////////////////
    BlinnPhongProgram::BlinnPhongProgram(const std::string& rootPath,
                                         ShaderLoader& loader,
                                         int numDirLights,
                                         int numPointLights,
                                         int numSpotLights,
                                         int numCascades,
                                         float shadowBiasMin,
                                         float shadowBiasMax,
                                         float pointShadowBias)
        : AbstractBlinnPhongProgram(rootPath, loader, false, numDirLights, numPointLights, numSpotLights, numCascades, shadowBiasMin, shadowBiasMax, pointShadowBias) {}

    ////////////////////////////////////////
    BlinnPhongProgramInstanced::BlinnPhongProgramInstanced(const std::string& rootPath,
                                                           ShaderLoader& loader,
                                                           int numDirLights,
                                                           int numPointLights,
                                                           int numSpotLights,
                                                           int numCascades,
                                                           float shadowBiasMin,
                                                           float shadowBiasMax,
                                                           float pointShadowBias)
        : AbstractBlinnPhongProgram(rootPath, loader, false, numDirLights, numPointLights, numSpotLights, numCascades, shadowBiasMin, shadowBiasMax, pointShadowBias) {}

    ////////////////////////////////////////
    AbstractDepthProgram::AbstractDepthProgram(const std::string& rootPath,
                                               ShaderLoader& loader,
                                               bool isInstanced,
                                               bool isOmni)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/depth.glsl",
                                { { "POE_APOS_LOC", ATTRIB_POS_LOC },
                                  { "POE_ULIGHT_MATRIX_LOC", LIGHT_MATRIX_LOC },
                                  { "POE_UMODEL_LOC", MODEL_MATRIX_LOC },
                                  { "POE_AMODEL_LOC", INSTANCED_MODEL_LOC },
                                  { "POE_INSTANCED", isInstanced ? 1 : 0 },
                                  { "POE_OMNI", isOmni ? 1 : 0 } }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/depth.glsl",
                                { { "POE_UFAR_PLANE_LOC", FAR_PLANE_LOC },
                                  { "POE_ULIGHT_POS_LOC", LIGHT_POS_LOC },
                                  { "POE_OMNI", isOmni ? 1 : 0 } }) } {}

    ////////////////////////////////////////
    DepthProgram::DepthProgram(const std::string& rootPath, ShaderLoader& loader)
        : AbstractDepthProgram(rootPath, loader, false, false) {}

    ////////////////////////////////////////
    DepthProgramInstanced::DepthProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : AbstractDepthProgram(rootPath, loader, true, false) {}

    ////////////////////////////////////////
    DepthOmniProgram::DepthOmniProgram(const std::string& rootPath, ShaderLoader& loader)
        : AbstractDepthProgram(rootPath, loader, false, true) {}

    ////////////////////////////////////////
    DepthOmniProgramInstanced::DepthOmniProgramInstanced(const std::string& rootPath, ShaderLoader& loader)
        : AbstractDepthProgram(rootPath, loader, true, true) {}

    ////////////////////////////////////////
    RealisticSkyboxProgram::RealisticSkyboxProgram(const std::string& rootPath,
                                                   ShaderLoader& loader,
                                                   float shaderPi, float iSteps, float jSteps)
        : mProgram{ loader.Load(GL_VERTEX_SHADER,
                                rootPath + "/shaders/realistic_skybox.glsl",
                                { { "POE_TRANSFORM_BLOCK_LOC", UniformBuffer::TRANSFORM_BLOCK_BINDING } }),
                    loader.Load(GL_FRAGMENT_SHADER,
                                rootPath + "/shaders/realistic_skybox.glsl",
                                { { "PI", shaderPi }, { "I_STEPS", iSteps }, { "J_STEPS", jSteps },
                                  { "POE_REALISTIC_SKYBOX_BLOCK_LOC", UniformBuffer::REALISTIC_SKYBOX_BLOCK_BINDING } }) },
         mShaderPI {shaderPi}, mShaderISteps{iSteps}, mShaderJSteps{jSteps} {}
}
