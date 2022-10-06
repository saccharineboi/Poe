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
    VertexBuffer::VertexBuffer(const std::vector<float>& vertices, int mode)
        : mMode{mode}, mNumElements{static_cast<int>(vertices.size())}
    {
        glGenBuffers(1, &mId);
        assert(mId != 0);

        glBindBuffer(GL_ARRAY_BUFFER, mId);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), mode);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
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
        glGenBuffers(1, &mId);
        assert(mId != 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), mode);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    VAO::VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos)
        : mNumIndices{ebo.GetNumElements()}
    {
        glGenVertexArrays(1, &mId);
        assert(mId != 0);

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
        assert(mId != 0 && source.size() > 0);

        const char* shaderSrc = source.c_str();
        glShaderSource(mId, 1, &shaderSrc, nullptr);
        glCompileShader(mId);

        int success = 0;
        glGetShaderiv(mId, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetShaderInfoLog(mId, 512, nullptr, infolog);
#ifdef _DEBUG
            std::fprintf(stderr, "[DEBUG] ERROR: %s\n", infolog);
#endif
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
        assert(mId != 0);

        for (const Shader& shader : shaders)
            glAttachShader(mId, shader.GetId());
        glLinkProgram(mId);

        int success = 0;
        glGetProgramiv(mId, GL_LINK_STATUS, &success);
        if (!success) {
            char infolog[512];
            glGetProgramInfoLog(mId, 512, nullptr, infolog);
#ifdef _DEBUG
            std::fprintf(stderr, "[DEBUG] ERROR: %s\n", infolog);
#endif
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
    Program CreateTextureSkyboxProgram(const std::string& rootPath, ShaderLoader& loader)
    {
        Shader& vshader = loader.Load(GL_VERTEX_SHADER, rootPath + "/shaders/texture_skybox.vert");
        Shader& fshader = loader.Load(GL_FRAGMENT_SHADER, rootPath + "/shaders/texture_skybox.frag");
        Program program{ vshader, fshader };

        program.Use();
            program.Uniform("uSkybox", 0);
        program.Halt();
        return program;
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
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
             0.0f,  0.5f, 0.0f,     0.5f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
        };

        return StaticMesh(vertices, indices, infos);
    }

    ////////////////////////////////////////
    StaticMesh CreateQuad()
    {
        std::vector<float> vertices {
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
            -0.5f,  0.5f, 0.0f,     0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,     1.0f, 1.0f
        };
        std::vector<unsigned> indices { 0, 1, 2, 2, 1, 3 };

        std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
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

        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        const float angleDelta = PI2 / static_cast<float>(numSegments);
        for (float angle = 0.0f; angle < PI2; angle += angleDelta) {
            float xPos = glm::cos(angle) * radius;
            float yPos = glm::sin(angle) * radius;
            float zPos = 0.0f;

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            vertices.push_back(xPos);
            vertices.push_back(yPos);
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
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
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
            std::fprintf(stderr, "[ERROR] ASSIMP: %s\n", importer.GetErrorString());
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
        std::printf("[DEBUG] Loaded %s (%d vertices and %d indices)\n", mPath.c_str(), numVertices, numIndices);
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

        std::vector<float> vertices;
        std::vector<unsigned> indices;
        std::vector<std::reference_wrapper<const Texture2D>> textures;

        for (int i = 0; i < static_cast<int>(mesh->mNumVertices); ++i) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        for (int i = 0; i < static_cast<int>(mesh->mNumFaces); ++i) {
            const aiFace& face = mesh->mFaces[i];
            for (int j = 0; j < static_cast<int>(face.mNumIndices); ++j)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material != nullptr)
            textures = Load2DTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

        static std::vector<VertexInfo> infos{
            { 0, 3, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(0) },
            { 1, 2, GL_FLOAT, static_cast<int>(5 * sizeof(float)), reinterpret_cast<const void*>(3 * sizeof(float)) }
        };

        return StaticMesh(vertices, indices, infos, textures);
    }

    // ////////////////////////////////////////
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
        }
        return textures;
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

            float sin_pitch = glm::sin(pitch);
            float cos_pitch = glm::cos(pitch);

            float sin_yaw = glm::sin(yaw);
            float cos_yaw = glm::cos(yaw);

            mDirection.x = cos_yaw * cos_pitch;
            mDirection.y = sin_pitch;
            mDirection.z = sin_yaw * cos_pitch;
        }
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
        glGenTextures(1, &mId);
        assert(mId != 0);

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

#ifdef _DEBUG
        std::printf("[DEBUG] Allocated %ld bytes for 2D texture %s\n", mWidth * mHeight * mNumChannels * sizeof(unsigned char) * (mParams.generateMipmaps ? 2 : 1), mUrl.c_str());
#endif
    }

    ////////////////////////////////////////
    Texture2D::Texture2D(const std::string& url, const Texture2DParams& params)
        : mUrl{url}, mParams{params}
    {
        assert(url.size() > 0);

        unsigned char* data = stbi_load(url.c_str(), &mWidth, &mHeight, &mNumChannels, 0);
        if (!data) {
#ifdef _DEBUG
            std::fprintf(stderr, "[DEBUG] ERROR: couldn't load %s\n", url.c_str());
#endif
            return;
        }

        switch (mNumChannels) {
            case 1:
                mParams.textureFormat = mParams.internalFormat = GL_RED;
                break;
            case 2:
                mParams.textureFormat = mParams.internalFormat = GL_RG;
                break;
            case 3:
                mParams.textureFormat = mParams.internalFormat = GL_RGB;
                break;
            case 4:
                mParams.textureFormat = mParams.internalFormat = GL_RGBA;
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
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumChannels{other.mNumChannels}, mUrl{other.mUrl}, mParams{other.mParams}
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
        glGenTextures(1, &mId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mId);
        for (const auto& face : faces) {
            unsigned char* data = stbi_load(face.second.data(), &mWidth, &mHeight, &mNumChannels, 0);
            if (!data) {
#ifdef _DEBUG
                std::fprintf(stderr, "[DEBUG] ERROR: couldn't load %s\n", face.second.data());
                return;
#endif
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
                    default:
#ifdef _DEBUG
                        std::fprintf(stderr, "[ERROR] Invalid cubemap face\n");
#endif
                        return 0;
                }
            }(face.first);

            glTexImage2D(texType, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    ////////////////////////////////////////
    Cubemap::Cubemap(Cubemap&& other)
        : mId{other.mId}, mWidth{other.mWidth}, mHeight{other.mHeight}, mNumChannels{other.mNumChannels}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Cubemap& Cubemap::operator=(Cubemap&& other)
    {
        glDeleteTextures(1, &mId);

        mId = other.mId;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mNumChannels = other.mNumChannels;

        other.mId = 0;
        return *this;
    }

    ////////////////////////////////////////
    Renderbuffer::Renderbuffer(int type, int width, int height)
        : mType{type}, mWidth{width}, mHeight{height}
    {
        glGenRenderbuffers(1, &mId);
        assert(mId != 0);

        glBindRenderbuffer(GL_RENDERBUFFER, mId);
            glRenderbufferStorage(GL_RENDERBUFFER, type, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
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
        glBindFramebuffer(GL_FRAMEBUFFER, mId);
        int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef _DEBUG
        if (status != GL_FRAMEBUFFER_COMPLETE)
            std::fprintf(stderr, "ERROR: framebuffer %u is not complete", mId);
#endif
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& colorAttachment)
    {
        glGenFramebuffers(1, &mId);
        assert(mId != 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment.GetId(), 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mColorAttachments.push_back(colorAttachment);
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(const Texture2D& colorAttachment, const Renderbuffer& rbo)
    {
        glGenFramebuffers(1, &mId);
        assert(mId != 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment.GetId(), 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.GetId());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mColorAttachments.push_back(colorAttachment);
        mRenderbuffers.push_back(rbo);
        Check();
    }

    ////////////////////////////////////////
    Framebuffer::Framebuffer(Framebuffer&& other)
        : mId{other.mId}, mColorAttachments{other.mColorAttachments}, mRenderbuffers{other.mRenderbuffers}
    {
        other.mId = 0;
    }

    ////////////////////////////////////////
    Framebuffer& Framebuffer::operator=(Framebuffer&& other)
    {
        glDeleteFramebuffers(1, &mId);

        mId = other.mId;
        mColorAttachments = other.mColorAttachments;
        mRenderbuffers = other.mRenderbuffers;

        other.mId = 0;
        return *this;
    }
}
