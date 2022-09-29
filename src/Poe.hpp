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

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_INLINE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <string>
#include <string_view>
#include <functional>

namespace Poe
{
    ////////////////////////////////////////
    inline constexpr float PI = 3.1415926f;
    inline constexpr float PI2 = 6.2831853f;
    inline constexpr float PIH = 1.5707963f;
    inline constexpr float R2D = 57.295779f;
    inline constexpr float D2R = 0.017453293f;

    ////////////////////////////////////////
    void APIENTRY GraphicsDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    ////////////////////////////////////////
    struct VertexBuffer
    {
    private:
        unsigned mId;
        int mMode;
        int mNumElements;

    public:
        VertexBuffer(const std::vector<float>& vertices, int mode);

        ~VertexBuffer() { glDeleteBuffers(1, &mId); }

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&&);
        VertexBuffer& operator=(VertexBuffer&&);

        void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

        unsigned GetId() const { return mId; }
        int GetMode() const { return mMode; }
        int GetNumElements() const { return mNumElements; }
    };

    ////////////////////////////////////////
    struct IndexBuffer
    {
    private:
        unsigned mId;
        int mMode;
        int mNumElements;

    public:
        IndexBuffer(const std::vector<unsigned>& indices, int mode);

        ~IndexBuffer() { glDeleteBuffers(1, &mId); }

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&&);
        IndexBuffer& operator=(IndexBuffer&&);

        void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

        unsigned GetId() const { return mId; }
        int GetMode() const { return mMode; }
        int GetNumElements() const { return mNumElements; }
    };

    ////////////////////////////////////////
    struct VertexInfo
    {
        int loc;
        int numElements;
        int dataType;
        int stride;
        const void* offset;
    };

    ////////////////////////////////////////
    struct VAO
    {
    private:
        unsigned mId;
        int mNumIndices;

    public:
        VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos);

        ~VAO() { glDeleteVertexArrays(1, &mId); }

        VAO(const VAO&) = delete;
        VAO& operator=(const VAO&) = delete;

        VAO(VAO&&);
        VAO& operator=(VAO&&);

        void Bind() const { glBindVertexArray(mId); }
        void UnBind() const { glBindVertexArray(0); }

        void Draw(int mode = GL_TRIANGLES) const { glDrawElements(mode, mNumIndices, GL_UNSIGNED_INT, nullptr); }

        unsigned GetId() const { return mId; }
        int GetNumIndices() const { return mNumIndices; }
    };

    ////////////////////////////////////////
    struct Shader
    {
    private:
        unsigned mId;
        int mType;

    public:
        Shader(int type, const std::string& source);

        ~Shader() { glDeleteShader(mId); }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&&);
        Shader& operator=(Shader&&);

        unsigned GetId() const { return mId; }
        int GetType() const { return mType; }
    };

    ////////////////////////////////////////
    struct Program
    {
    private:
        unsigned mId;
        std::unordered_map<std::string, int> mUniforms;

        int FindUniform(std::string_view name)
        {
            auto iter = mUniforms.find(name.data());
            if (iter == mUniforms.end()) {
                int loc = glGetUniformLocation(mId, name.data());
                if (-1 == loc) {
#ifdef _DEBUG
                    std::fprintf(stderr, "[DEBUG] ERROR: %s not found\n", name.data());
#endif
                }
                else
                    mUniforms.insert(std::pair(name, loc));
                return loc;
            }
            return iter->second;
        }

    public:
        Program(std::initializer_list<std::reference_wrapper<const Shader>> shaders);

        ~Program() { glDeleteProgram(mId); }

        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        Program(Program&&);
        Program& operator=(Program&&);

        void Use() const { glUseProgram(mId); }
        void Halt() const { glUseProgram(0); }

        void Uniform(std::string_view name, int x)
        { glUniform1i(FindUniform(name), x); }

        void Uniform(std::string_view name, int x, int y)
        { glUniform2i(FindUniform(name), x, y); }

        void Uniform(std::string_view name, int x, int y, int z)
        { glUniform3i(FindUniform(name), x, y, z); }

        void Uniform(std::string_view name, int x, int y, int z, int w)
        { glUniform4i(FindUniform(name), x, y, z, w); }

        void Uniform(std::string_view name, float x)
        { glUniform1f(FindUniform(name), x); }

        void Uniform(std::string_view name, float x, float y)
        { glUniform2f(FindUniform(name), x, y); }

        void Uniform(std::string_view name, float x, float y, float z)
        { glUniform3f(FindUniform(name), x, y, z); }

        void Uniform(std::string_view name, float x, float y, float z, float w)
        { glUniform4f(FindUniform(name), x, y, z, w); }

        void Uniform(std::string_view name, const glm::vec2& v)
        { glUniform2fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(std::string_view name, const glm::vec3& v)
        { glUniform3fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(std::string_view name, const glm::vec4& v)
        { glUniform4fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(std::string_view name, const glm::mat3& m)
        { glUniformMatrix3fv(FindUniform(name), 1, GL_FALSE, glm::value_ptr(m)); }

        void Uniform(std::string_view name, const glm::mat4& m)
        { glUniformMatrix4fv(FindUniform(name), 1, GL_FALSE, glm::value_ptr(m)); }

        unsigned GetId() const { return mId; }
    };

    ////////////////////////////////////////
    struct ShaderLoader
    {
    private:
        // key: path, data: shader content
        std::unordered_map<std::string, Shader> mShaders;

    public:
        Shader& Load(int type, std::string_view shaderUrl);
    };

    ////////////////////////////////////////
    Program CreateBasicProgram(const std::string& rootPath, ShaderLoader&);
    Program CreateEmissiveColorProgram(const std::string& rootPath, ShaderLoader&);
    Program CreateEmissiveTextureProgram(const std::string& rootPath, ShaderLoader&);

    ////////////////////////////////////////
    struct PostProcessProgram
    {
    private:
        Program mProgram;

    public:
        PostProcessProgram(const std::string& rootPath, ShaderLoader&);

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }
        void Draw() const { glDrawArrays(GL_TRIANGLES, 0, 6); }
    };

    ////////////////////////////////////////
    struct Texture2DParams
    {
        int textureFormat = GL_RGB;
        int internalFormat = GL_RGB;
        bool generateMipmaps = true;
        float maxAnisotropy = 16.0f;
        int wrapS = GL_REPEAT;
        int wrapT = GL_REPEAT;
        int minF = GL_LINEAR_MIPMAP_LINEAR;
        int magF = GL_LINEAR;
        int type = GL_UNSIGNED_BYTE;
    };

    ////////////////////////////////////////
    struct Texture2D
    {
    private:
        unsigned mId;

        int mWidth;
        int mHeight;
        int mNumChannels;
        std::string mUrl;

        Texture2DParams mParams;

        template <typename T>
        void Create(T* data);

    public:
        Texture2D(const std::string& url, const Texture2DParams&);

        template <typename T>
        Texture2D(T* data, int width, int height, int numChannels, const Texture2DParams&);
~Texture2D() { glDeleteTextures(1, &mId); }

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D(Texture2D&&);
        Texture2D& operator=(Texture2D&&);

        unsigned GetId() const { return mId; }

        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        int GetNumChannels() const { return mNumChannels; }
        std::string GetUrl() const { return mUrl; }

        int GetTextureFormat() const { return mParams.textureFormat; }
        int GetInternalFormat() const { return mParams.internalFormat; }
        bool HasMipmaps() const { return mParams.generateMipmaps; }
        float GetMaxAnisotropy() const { return mParams.maxAnisotropy; }
        int GetWrapS() const { return mParams.wrapS; }
        int GetWrapT() const { return mParams.wrapT; }
        int GetMinF() const { return mParams.minF; }
        int GetMagF() const { return mParams.magF; }
        int GetType() const { return mParams.type; }

        void Bind(int loc = 0) const
        {
            glActiveTexture(GL_TEXTURE0 + loc);
            glBindTexture(GL_TEXTURE_2D, mId);
        }

        void UnBind(int loc = 0) const
        {
            glActiveTexture(GL_TEXTURE0 + loc);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    };

    ////////////////////////////////////////
    Texture2D CreateCheckerboardTexture2D(const glm::vec3& color0 = glm::vec3(0.75f),
                                          const glm::vec3& color1 = glm::vec3(0.25f));
    Texture2D CreateFramebufferTexture2D(int width, int height);

    ////////////////////////////////////////
    struct Texture2DLoader
    {
    private:
        std::unordered_map<std::string, Texture2D> mTextures;

    public:
        Texture2D& Load(const std::string& url, const Texture2DParams&);
    };

    ////////////////////////////////////////
    struct Renderbuffer
    {
    private:
        unsigned mId;
        int mType;
        int mWidth, mHeight;

    public:
        Renderbuffer(int type, int width, int height);

        ~Renderbuffer() { glDeleteRenderbuffers(1, &mId); }

        Renderbuffer(const Renderbuffer&) = delete;
        Renderbuffer& operator=(const Renderbuffer&) = delete;

        Renderbuffer(Renderbuffer&&);
        Renderbuffer& operator=(Renderbuffer&&);

        void Bind() const { glBindRenderbuffer(GL_RENDERBUFFER, mId); }
        void UnBind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

        unsigned GetId() const { return mId; }
        int GetType() const { return mType; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
    };

    ////////////////////////////////////////
    struct Framebuffer
    {
    private:
        unsigned mId;
        std::vector<std::reference_wrapper<const Texture2D>> mColorAttachments;
        std::vector<std::reference_wrapper<const Renderbuffer>> mRenderbuffers;

    public:
        explicit Framebuffer(const Texture2D& colorAttachment);
        Framebuffer(const Texture2D& colorAttachment, const Renderbuffer& rbo);

        ~Framebuffer() { glDeleteFramebuffers(1, &mId); }

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        Framebuffer(Framebuffer&&);
        Framebuffer& operator=(Framebuffer&&);

        void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, mId); }
        void UnBind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
        bool Check() const;

        unsigned GetId() const { return mId; }
        std::size_t GetColorAttachmentsCount() const { return mColorAttachments.size(); }
        std::size_t GetRenderbuffersCount() const { return mRenderbuffers.size(); }
    };

    ////////////////////////////////////////
    struct StaticMesh
    {
    private:
        VertexBuffer mVbo;
        IndexBuffer mEbo;
        VAO mVao;
        std::vector<std::reference_wrapper<const Texture2D>> mTextures;

    public:
        StaticMesh(const std::vector<float>& vertices,
                   const std::vector<unsigned>& indices,
                   const std::vector<VertexInfo>& infos)
            : mVbo(vertices, GL_STATIC_DRAW),
              mEbo(indices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos) {}

        StaticMesh(const std::vector<float>& vertices,
                   const std::vector<unsigned>& indices,
                   const std::vector<VertexInfo>& infos,
                   const std::vector<std::reference_wrapper<const Texture2D>>& textures)
            : mVbo(vertices, GL_STATIC_DRAW),
              mEbo(indices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mTextures{textures} {}

        void Bind() const { mVao.Bind(); }
        void UnBind() const { mVao.UnBind(); }
        void Draw(int mode = GL_TRIANGLES) const { mVao.Draw(mode); }
        void AddTexture(const Texture2D& t) { mTextures.push_back(t); }

        void BindTextures() const
        { int i{}; for (const Texture2D& t : mTextures) t.Bind(i++); }
        void UnBindTextures() const
        { int i{}; for (const Texture2D& t : mTextures) t.UnBind(i++); }
    };

    ////////////////////////////////////////
    StaticMesh CreateColoredTriangle();
    StaticMesh CreateColoredQuad();
    StaticMesh CreateColoredCircle(float radius, int numSegments);

    ////////////////////////////////////////
    StaticMesh CreateTriangle();
    StaticMesh CreateQuad();
    StaticMesh CreateCircle(float radius, int numSegments);
    StaticMesh CreateCube();
    StaticMesh CreateGrid(int numX, int numZ);
    StaticMesh CreatePyramid();

    ////////////////////////////////////////
    struct StaticModel
    {
    private:
        std::string mPath;
        std::string mDirectory;
        std::vector<StaticMesh> mMeshes;
        Texture2DLoader& mTexture2DLoader;

        void Load();
        void LoadNode(aiNode* node, const aiScene* scene);
        StaticMesh LoadStaticMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<std::reference_wrapper<const Texture2D>> Load2DTextures(aiMaterial* material, aiTextureType type, std::string_view typeName);

    public:
        StaticModel(const std::string& modelPath, Texture2DLoader& texture2DLoader)
            : mPath{modelPath}, mTexture2DLoader{texture2DLoader} { Load(); }

        void Draw(int mode = GL_TRIANGLES) const
        {
            for (const StaticMesh& staticMesh : mMeshes) {
                staticMesh.Bind();
                staticMesh.BindTextures();
                staticMesh.Draw(mode);
            }
        }

        std::string GetPath() const { return mPath; }
        std::string GetDirectory() const { return mDirectory; }
    };

    ////////////////////////////////////////
    struct FirstPersonCameraState
    {
        bool movingForward = false;
        bool movingBackward = false;
        bool movingLeft = false;
        bool movingRight = false;
        bool movingUp = false;
        bool movingDown = false;
    };

    ////////////////////////////////////////
    struct FirstPersonCameraInputConfig
    {
        int moveForwardKey = GLFW_KEY_W;
        int moveBackwardKey = GLFW_KEY_S;
        int moveLeftKey = GLFW_KEY_A;
        int moveRightKey = GLFW_KEY_D;
        int moveUpKey = GLFW_KEY_Q;
        int moveDownKey = GLFW_KEY_E;
    };

    ////////////////////////////////////////
    struct FirstPersonCamera
    {
        FirstPersonCameraState mState;
        FirstPersonCameraInputConfig mInputConfig;

        bool mIsMouseCaptured = false;

        float mFovy = PIH;
        float mAspectRatio = 16.0f / 9.0f;
        float mNear = 0.3f;
        float mFar = 1000.0f;

        float mSpeed = 10.0f;
        float mSensitivity = 0.0025f;
        float mSmoothness = 10.0f;

        glm::mat4 mProjection = glm::mat4(1.0f);
        glm::mat4 mView = glm::mat4(1.0f);

        glm::vec3 mPosition = glm::vec3(0.0f, 1.0f, 10.0f);
        glm::vec3 mDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 mTargetPosition = mPosition;

        void UpdateInputConfig(int key, int action);
        void UpdateDirection(float mouseX, float mouseY);
        void Update(float dt);
    };
}
