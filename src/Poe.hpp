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

#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <string>
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
    struct ObjectGL
    {
    protected:
        unsigned mId;

    public:
        ObjectGL() {}
        explicit ObjectGL(unsigned id) : mId{id} {}

        virtual ~ObjectGL() {}

        unsigned GetId() const { return mId; }

        ObjectGL(const ObjectGL&) = delete;
        ObjectGL& operator=(const ObjectGL&) = delete;
    };

    ////////////////////////////////////////
    struct BufferGL : public ObjectGL
    {
    private:
        unsigned CreateId();

    protected:
        int mMode;
        int mNumElements;

    public:
        BufferGL(int mode, int numElements);
        BufferGL(int id, int mode, int numElements);

        virtual ~BufferGL() { glDeleteBuffers(1, &mId); }

        BufferGL(const BufferGL&) = delete;
        BufferGL& operator=(const BufferGL&) = delete;

        int GetMode() const { return mMode; }
        int GetNumElements() const { return mNumElements; }
    };

    ////////////////////////////////////////
    struct VertexBuffer : public BufferGL
    {
        VertexBuffer(const std::vector<float>& vertices, int mode);

        VertexBuffer(VertexBuffer&&);
        VertexBuffer& operator=(VertexBuffer&&);

        void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
    };

    ////////////////////////////////////////
    struct IndexBuffer : public BufferGL
    {
        IndexBuffer(const std::vector<unsigned>& indices, int mode);

        IndexBuffer(IndexBuffer&&);
        IndexBuffer& operator=(IndexBuffer&&);

        void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
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
    struct VAO : public ObjectGL
    {
    private:
        int mNumIndices;

        unsigned CreateId();

    public:
        VAO(const VertexBuffer& vbo, const IndexBuffer& ebo, const std::vector<VertexInfo>& infos);

        ~VAO() { glDeleteVertexArrays(1, &mId); }

        VAO(VAO&&);
        VAO& operator=(VAO&&);

        void Bind() const { glBindVertexArray(mId); }
        void UnBind() const { glBindVertexArray(0); }

        void Draw(int mode = GL_TRIANGLES) const { glDrawElements(mode, mNumIndices, GL_UNSIGNED_INT, nullptr); }
    };

    ////////////////////////////////////////
    struct Shader : public ObjectGL
    {
    private:
        int mType;

    public:
        Shader(int type, const std::string& source);

        ~Shader() { glDeleteShader(mId); }

        Shader(Shader&&);
        Shader& operator=(Shader&&);

        int GetType() const { return mType; }
    };

    ////////////////////////////////////////
    struct Program : public ObjectGL
    {
    private:
        std::unordered_map<std::string, int> mUniforms;

        int FindUniform(const std::string& name)
        {
            auto iter = mUniforms.find(name);
            if (iter == mUniforms.end()) {
                int loc = glGetUniformLocation(mId, name.c_str());
                if (-1 == loc)
                    std::fprintf(stderr, "[DEBUG] ERROR: %s not found\n", name.c_str());
                else
                    mUniforms.insert(std::pair(name, loc));
                return loc;
            }
            return iter->second;
        }

    public:
        Program(std::initializer_list<std::reference_wrapper<const Shader>> shaders);

        ~Program() { glDeleteProgram(mId); }

        Program(Program&&);
        Program& operator=(Program&&);

        void Use() const { glUseProgram(mId); }
        void Halt() const { glUseProgram(0); }

        void Uniform(const std::string& name, int x)
        { glUniform1i(FindUniform(name), x); }

        void Uniform(const std::string& name, int x, int y)
        { glUniform2i(FindUniform(name), x, y); }

        void Uniform(const std::string& name, int x, int y, int z)
        { glUniform3i(FindUniform(name), x, y, z); }

        void Uniform(const std::string& name, int x, int y, int z, int w)
        { glUniform4i(FindUniform(name), x, y, z, w); }

        void Uniform(const std::string& name, float x)
        { glUniform1f(FindUniform(name), x); }

        void Uniform(const std::string& name, float x, float y)
        { glUniform2f(FindUniform(name), x, y); }

        void Uniform(const std::string& name, float x, float y, float z)
        { glUniform3f(FindUniform(name), x, y, z); }

        void Uniform(const std::string& name, float x, float y, float z, float w)
        { glUniform4f(FindUniform(name), x, y, z, w); }

        void Uniform(const std::string& name, const glm::vec2& v)
        { glUniform2fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(const std::string& name, const glm::vec3& v)
        { glUniform3fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(const std::string& name, const glm::vec4& v)
        { glUniform4fv(FindUniform(name), 1, glm::value_ptr(v)); }

        void Uniform(const std::string& name, const glm::mat3& m)
        { glUniformMatrix3fv(FindUniform(name), 1, GL_FALSE, glm::value_ptr(m)); }

        void Uniform(const std::string& name, const glm::mat4& m)
        { glUniformMatrix4fv(FindUniform(name), 1, GL_FALSE, glm::value_ptr(m)); }
    };

    ////////////////////////////////////////
    struct ShaderLoader
    {
    private:
        // key: path, data: shader content
        std::unordered_map<std::string, Shader> mShaders;

    public:
        Shader& Load(int type, const std::string& shaderUrl);
    };

    ////////////////////////////////////////
    Program CreateBasicProgram(const std::string& rootPath, ShaderLoader&);
    Program CreateEmissiveColorProgram(const std::string& rootPath, ShaderLoader&);
    Program CreateEmissiveTextureProgram(const std::string& rootPath, ShaderLoader&);

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
    struct Texture2D : public ObjectGL
    {
    private:
        int mWidth;
        int mHeight;
        int mNumChannels;
        std::string mUrl;

        Texture2DParams mParams;

        template <typename T>
        void Create(T* data);

        unsigned CreateId();

    public:
        Texture2D(const std::string& url, const Texture2DParams&);

        template <typename T>
        Texture2D(T* data, int width, int height, int numChannels, const Texture2DParams&);

        ~Texture2D() { glDeleteTextures(1, &mId); }

        Texture2D(Texture2D&&);
        Texture2D& operator=(Texture2D&&);

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
    Texture2D CreateCheckerboardTexture2D();

    ////////////////////////////////////////
    struct Texture2DLoader
    {
    private:
        std::unordered_map<std::string, Texture2D> mTextures;

    public:
        Texture2D& Load(const std::string& url, const Texture2DParams&);
    };

    ////////////////////////////////////////
    struct StaticMesh
    {
    private:
        VertexBuffer mVbo;
        IndexBuffer mEbo;
        VAO mVao;

    public:
        StaticMesh(const std::vector<float>& vertices,
                   const std::vector<unsigned>& indices,
                   const std::vector<VertexInfo>& infos);

        ~StaticMesh() {}

        void Bind() const { mVao.Bind(); }
        void UnBind() const { mVao.UnBind(); }
        void Draw(int mode = GL_TRIANGLES) const { mVao.Draw(mode); }
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
        float mNear = 0.1f;
        float mFar = 100.0f;

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
