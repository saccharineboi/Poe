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

#include "Constants.hpp"
#include "UI.hpp"

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
#include <utility>
#include <memory>

#include <cstring>

namespace Poe
{
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
        VertexBuffer(int numElements, int mode);
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

        float* GetWritePtr() const
        { return reinterpret_cast<float*>(glMapNamedBuffer(mId, GL_WRITE_ONLY)); }
        int Unmap() const { return glUnmapNamedBuffer(mId); }

        void Modify(int offset, int size, const void* data) const
        { glNamedBufferSubData(mId, offset, size, data); }
    };

    ////////////////////////////////////////
    struct IndexBuffer
    {
    private:
        unsigned mId;
        int mMode;
        int mNumElements;

    public:
        IndexBuffer(int numElements, int mode);
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

        unsigned* GetWritePtr() const
        { return reinterpret_cast<unsigned*>(glMapNamedBuffer(mId, GL_WRITE_ONLY)); }
        int Unmap() const { return glUnmapNamedBuffer(mId); }

        void Modify(int offset, int size, const void* data) const
        { glNamedBufferSubData(mId, offset, size, data); }
    };

    ////////////////////////////////////////
    struct UniformBuffer
    {
    private:
        unsigned mId;
        int mSize;
        int mMode;
        int mBindLoc;

    public:
        UniformBuffer(int size, int mode, int bindLoc);

        ~UniformBuffer() { glDeleteBuffers(1, &mId); }

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        UniformBuffer(UniformBuffer&&);
        UniformBuffer& operator=(UniformBuffer&&);

        void Bind() const { glBindBuffer(GL_UNIFORM_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

        void TurnOn() const { glBindBufferBase(GL_UNIFORM_BUFFER, mBindLoc, mId); }
        void TurnOff() const { glBindBufferBase(GL_UNIFORM_BUFFER, mBindLoc, 0); }

        void Modify(int offset, int size, const void* data) const
        { glNamedBufferSubData(mId, offset, size, data); }

        unsigned GetId() const { return mId; }
        int GetSize() const { return mSize; }
        int GetMode() const { return mMode; }
        int GetBindLoc() const { return mBindLoc; }
    };

    ////////////////////////////////////////
    struct FogUB__DATA
    {
        alignas(16) float color[3];
        alignas(16) float distance;
        alignas(4) float exponent;

        glm::vec3 GetColor() const { return glm::vec3(color[0], color[1], color[2]); }
        void SetColor(const glm::vec3& c) { std::memcpy(color, glm::value_ptr(c), 12); }
    };

    ////////////////////////////////////////
    struct FogUB
    {
    private:
        UniformBuffer mBuffer;
        FogUB__DATA mData;

    public:
        FogUB(const glm::vec3& color, float distance, float exponent);

        const UniformBuffer& Buffer() const { return mBuffer; }

        glm::vec3 GetColor() const { return mData.GetColor(); }
        float GetDistance() const { return mData.distance; }
        float GetExponent() const { return mData.exponent; }

        void SetColor(const glm::vec3& color) { mData.SetColor(color); }
        void SetDistance(float distance) { mData.distance = distance; }
        void SetExponent(float exponent) { mData.exponent = exponent; }

        void Update() const { mBuffer.Modify(0, sizeof(FogUB__DATA), &mData); }
    };

    ////////////////////////////////////////
    struct FirstPersonCamera;

    ////////////////////////////////////////
    struct TransformUB__DATA
    {
        alignas(64) float projection_data[16];
        alignas(64) float view_data[16];
        alignas(64) float projView_data[16];
        alignas(16) float camDir_data[3];

        void SetProjectionData(const glm::mat4& projectionMatrix)
        { std::memcpy(projection_data, glm::value_ptr(projectionMatrix), 64); }

        void SetViewData(const glm::mat4& viewMatrix)
        { std::memcpy(view_data, glm::value_ptr(viewMatrix), 64); }

        void SetProjViewData(const glm::mat4& projViewMatrix)
        { std::memcpy(projView_data, glm::value_ptr(projViewMatrix), 64); }

        void SetCamDirData(const glm::vec3& camDir)
        { std::memcpy(camDir_data, glm::value_ptr(camDir), 12); }
    };

    ////////////////////////////////////////
    struct TransformUB
    {
    private:
        UniformBuffer mBuffer;
        TransformUB__DATA mData;

    public:
        TransformUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        void SetProjectionMatrix(const glm::mat4& projectionMatrix)
        { mData.SetProjectionData(projectionMatrix); }

        void SetViewMatrix(const glm::mat4& viewMatrix)
        { mData.SetViewData(viewMatrix); }

        void SetProjViewMatrix(const glm::mat4& projViewMatrix)
        { mData.SetProjViewData(projViewMatrix); }

        void SetCamDir(const glm::vec3& camDir)
        { mData.SetCamDirData(camDir); }

        void Update() const { mBuffer.Modify(0, sizeof(TransformUB__DATA), &mData); }
        void Set(const FirstPersonCamera& camera);
    };

    ////////////////////////////////////////
    struct PbrLightMaterial
    {
        glm::vec3 mAlbedo;
        float mMetallic;
        float mRoughness;
        float mAo;
    };

    ////////////////////////////////////////
    struct PbrLightMaterial__DATA
    {
        alignas(16) float albedo[3];
        alignas(4) float metallic;
        alignas(4) float roughness;
        alignas(4) float ao;

        void SetAlbedo(const glm::vec3& v)
        { std::memcpy(albedo, glm::value_ptr(v), 12); }
    };

    ////////////////////////////////////////
    struct PbrLightMaterialUB
    {
    private:
        UniformBuffer mBuffer;
        PbrLightMaterial__DATA mData;

    public:
        PbrLightMaterialUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        void SetAlbedo(const glm::vec3& albedo) { mData.SetAlbedo(albedo); }
        void SetMetallic(float metallic) { mData.metallic = metallic; }
        void SetRoughness(float roughness) { mData.roughness = roughness; }
        void SetAO(float ao) { mData.ao = ao; }

        void Update() const { mBuffer.Modify(0, sizeof(PbrLightMaterial__DATA), &mData); }
        void Set(const PbrLightMaterial&);
    };

    ////////////////////////////////////////
    struct DirLight
    {
        glm::vec3 mColor;
        glm::vec3 mDirection;
        float mIntensity;
    };

    ////////////////////////////////////////
    struct DirLight__DATA
    {
        alignas(16) float color[3];
        alignas(16) float direction[3];
        alignas(4) float intensity;
    };

    ////////////////////////////////////////
    inline constexpr int NUM_DIR_LIGHTS = 2;
    struct alignas(16) DirLightListElem__DATA
    {
        DirLight__DATA data;

        void SetColor(const glm::vec3& color)
        { std::memcpy(data.color, glm::value_ptr(color), 12); }

        void SetDirection(const glm::vec3& dir)
        { std::memcpy(data.direction, glm::value_ptr(dir), 12); }

        void SetIntensity(float intensity)
        { data.intensity = intensity; }
    };

    ////////////////////////////////////////
    struct DirLightUB
    {
    private:
        UniformBuffer mBuffer;
        DirLightListElem__DATA mLightsData[NUM_DIR_LIGHTS];

    public:
        DirLightUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        inline static constexpr int DATA_SIZE = sizeof(DirLightListElem__DATA) * NUM_DIR_LIGHTS;

        void SetColor(int ind, const glm::vec3& color)
        { mLightsData[ind].SetColor(color); }

        void SetDirection(int ind, const glm::vec3& dir)
        { mLightsData[ind].SetDirection(dir); }

        void SetIntensity(int ind, float intensity)
        { mLightsData[ind].SetIntensity(intensity); }

        void Update() const
        { mBuffer.Modify(0, DATA_SIZE, mLightsData); }

        void Set(int ind, const DirLight& dirLight);
    };

    ////////////////////////////////////////
    struct VertexInfo
    {
        int loc;
        int numElements;
        int dataType;
        int stride;
        int offset;
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

        void Draw(int mode = GL_TRIANGLES) const
        { glDrawElements(mode, mNumIndices, GL_UNSIGNED_INT, nullptr); }

        void DrawInstanced(int mode = GL_TRIANGLES, int numInstances = 1) const
        { glDrawElementsInstanced(mode, mNumIndices, GL_UNSIGNED_INT, nullptr, numInstances); }

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
                if (-1 == loc)
                    DebugUI::PushLog(stderr, "[DEBUG] ERROR: %s not found\n", name.data());
                else
                    mUniforms.insert(std::make_pair(name, loc));
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

    ////////////////////////////////////////
    struct PostProcessProgram
    {
    private:
        Program mProgram;

    public:
        float mGamma{2.2f};
        float mExposure{1.0f};
        float mGrayscaleWeight{0.0f};
        float mKernelWeight{0.0f};

        PostProcessProgram(const std::string& rootPath, ShaderLoader&);

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }
        void Draw() const { glDrawArrays(GL_TRIANGLES, 0, 6); }

        static inline constexpr int GRAYSCALE_WEIGHT_LOC = 0;
        static inline constexpr int KERNEL_WEIGHT_LOC = 1;
        static inline constexpr int SCREEN_TEXTURE_LOC = 2;
        static inline constexpr int GAMMA_LOC = 3;
        static inline constexpr int EXPOSURE_LOC = 4;
        static inline constexpr int KERNEL_LOC = 5;

        void UpdateGrayscaleWeight() const { glUniform1f(GRAYSCALE_WEIGHT_LOC, mGrayscaleWeight); }
        void UpdateKernelWeight() const { glUniform1f(KERNEL_WEIGHT_LOC, mKernelWeight); }
        void UpdateGamma() const { glUniform1f(GAMMA_LOC, mGamma); }
        void UpdateExposure() const { glUniform1f(EXPOSURE_LOC, mExposure); }

        void SetKernel(const glm::mat3& m) const
        { glUniformMatrix3fv(KERNEL_LOC, 1, GL_FALSE, glm::value_ptr(m)); }

        void SetIdentityKernel() const
        { SetKernel(glm::mat3{0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f}); }

        void SetSharpenKernel() const
        { SetKernel(glm::mat3{-1.0f, -1.0f, -1.0f,
                              -1.0f,  9.0f, -1.0f,
                              -1.0f, -1.0f, -1.0f}); }

        void SetEdgeDetectKernel() const
        { SetKernel(glm::mat3{1.0f,  1.0f, 1.0f,
                              1.0f, -8.0f, 1.0f,
                              1.0f,  1.0f, 1.0f}); }
    };

    ////////////////////////////////////////
    struct Texture2DParams
    {
        int textureFormat = GL_RGB;
        int internalFormat = GL_RGB8;
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

        int mNumMipmaps;

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

        int GetNumMipmaps() const { return mNumMipmaps; }

        void Bind(int loc = 0) const { glBindTextureUnit(loc, mId); }
        void UnBind(int loc = 0) const { glBindTextureUnit(loc, 0); }
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
    enum class CubemapFace { Front, Back, Left, Right, Top, Bottom };

    ////////////////////////////////////////
    struct Cubemap
    {
    private:
        unsigned mId;
        int mWidth;
        int mHeight;
        int mNumChannels;
        int mNumMipmaps;

    public:
        Cubemap(std::initializer_list<std::pair<CubemapFace, std::string_view>> faces);

        ~Cubemap() { glDeleteTextures(1, &mId); }

        Cubemap(const Cubemap&) = delete;
        Cubemap& operator=(const Cubemap&) = delete;

        Cubemap(Cubemap&&);
        Cubemap& operator=(Cubemap&&);

        void Bind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, mId); }
        void Unbind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

        unsigned GetId() const { return mId; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        int GetNumChannels() const { return mNumChannels; }
        int GetNumMipmaps() const { return mNumMipmaps; }
    };

    ////////////////////////////////////////
    Cubemap CreateUlukaiCoronaSkybox(const std::string& root);
    Cubemap CreateUlukaiRedEclipseSkybox(const std::string& root);
    Cubemap CreateCloudySkybox(const std::string& root);

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
    struct RenderbufferMultiSample
    {
    private:
        unsigned mId;
        int mType;
        int mWidth;
        int mHeight;
        int mNumSamples;

    public:
        RenderbufferMultiSample(int type, int width, int height, int numSamples);

        ~RenderbufferMultiSample() { glDeleteRenderbuffers(1, &mId); }

        RenderbufferMultiSample(const RenderbufferMultiSample&) = delete;
        RenderbufferMultiSample& operator=(const RenderbufferMultiSample&) = delete;

        RenderbufferMultiSample(RenderbufferMultiSample&&);
        RenderbufferMultiSample& operator=(RenderbufferMultiSample&&);

        unsigned GetId() const { return mId; }
        int GetType() const { return mType; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        int GetNumSamples() const { return mNumSamples;  }

        void Bind() const { glBindRenderbuffer(GL_RENDERBUFFER, mId); }
        void UnBind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
    };

    ////////////////////////////////////////
    struct Texture2DMultiSample
    {
    private:
        unsigned mId;
        int mWidth;
        int mHeight;
        int mType;
        int mNumSamples;

    public:
        Texture2DMultiSample(int width, int height, int type, int numSamples);

        ~Texture2DMultiSample() { glDeleteTextures(1, &mId); }

        Texture2DMultiSample(const Texture2DMultiSample&) = delete;
        Texture2DMultiSample& operator=(const Texture2DMultiSample&) = delete;

        Texture2DMultiSample(Texture2DMultiSample&&);
        Texture2DMultiSample& operator=(Texture2DMultiSample&&);

        unsigned GetId() const { return mId; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        int GetType() const { return mType; }
        int GetNumSamples() const { return mNumSamples; }

        void Bind() const { glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mId); }
        void UnBind() const { glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0); }
    };

    ////////////////////////////////////////
    struct Framebuffer
    {
    private:
        unsigned mId;

    public:
        explicit Framebuffer(const Texture2D&);
        Framebuffer(const Texture2D&, const Renderbuffer&);
        Framebuffer(const Texture2DMultiSample&, const RenderbufferMultiSample&);

        ~Framebuffer() { glDeleteFramebuffers(1, &mId); }

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        Framebuffer(Framebuffer&&);
        Framebuffer& operator=(Framebuffer&&);

        void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, mId); }
        void UnBind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
        bool Check() const;

        void Blit(int width, int height) const
        { glBlitNamedFramebuffer(mId, 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST); }

        void Blit(const Framebuffer& fb, int width, int height) const
        { glBlitNamedFramebuffer(mId, fb.GetId(), 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST); }

        unsigned GetId() const { return mId; }
    };

    ////////////////////////////////////////
    struct StaticMesh
    {
    private:
        VertexBuffer mVbo;
        IndexBuffer mEbo;
        VAO mVao;
        std::vector<std::reference_wrapper<const Texture2D>> mTextures;
        std::unique_ptr<VertexBuffer> mModelMatrixBuffer;
        int mNumInstances;

        void ReconfigureMatrixBuffer();

    public:
        StaticMesh(int numInstances,
                   const std::vector<float>& vertices,
                   const std::vector<unsigned>& indices,
                   const std::vector<VertexInfo>& infos)
            : mVbo(vertices, GL_STATIC_DRAW),
              mEbo(indices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mModelMatrixBuffer{new VertexBuffer(16, GL_DYNAMIC_DRAW)},
              mNumInstances{numInstances}
        { CreateInstances(mNumInstances); }

        StaticMesh(int numInstances,
                   const std::vector<float>& vertices,
                   const std::vector<unsigned>& indices,
                   const std::vector<VertexInfo>& infos,
                   const std::vector<std::reference_wrapper<const Texture2D>>& textures)
            : mVbo(vertices, GL_STATIC_DRAW),
              mEbo(indices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mTextures{textures},
              mModelMatrixBuffer{new VertexBuffer(16, GL_DYNAMIC_DRAW)},
              mNumInstances{numInstances}
        { CreateInstances(mNumInstances); }

        StaticMesh(int numInstances,
                   int numVertices,
                   int numIndices,
                   const std::vector<VertexInfo>& infos)
            : mVbo(numVertices, GL_STATIC_DRAW),
              mEbo(numIndices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mModelMatrixBuffer{new VertexBuffer(16, GL_DYNAMIC_DRAW)},
              mNumInstances{numInstances}
        { CreateInstances(mNumInstances); }

        StaticMesh(int numInstances,
                   int numVertices,
                   int numIndices,
                   const std::vector<VertexInfo>& infos,
                   const std::vector<std::reference_wrapper<const Texture2D>>& textures)
            : mVbo(numVertices, GL_STATIC_DRAW),
              mEbo(numIndices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mTextures{textures},
              mModelMatrixBuffer{new VertexBuffer(16, GL_DYNAMIC_DRAW)},
              mNumInstances{numInstances}
        { CreateInstances(mNumInstances); }

        void Bind() const { mVao.Bind(); }
        void UnBind() const { mVao.UnBind(); }

        void Draw(int mode = GL_TRIANGLES) const
        { mVao.Draw(mode); }

        void DrawInstanced(int mode = GL_TRIANGLES) const
        { mVao.DrawInstanced(mode, mNumInstances); }

        void AddTexture(const Texture2D& t) { mTextures.push_back(t); }
        void AddTextures(const std::vector<std::reference_wrapper<const Texture2D>>& textures) { std::ranges::copy(textures, std::back_inserter(mTextures)); }

        void BindTextures() const
        { int i{}; for (const Texture2D& t : mTextures) t.Bind(i++); }
        void UnBindTextures() const
        { int i{}; for (const Texture2D& t : mTextures) t.UnBind(i++); }

        int GetNumVertices() const { return mVbo.GetNumElements(); }
        int GetNumIndices() const { return mEbo.GetNumElements(); }

        float* GetVboWritePtr() const { return mVbo.GetWritePtr(); }
        unsigned* GetEboWritePtr() const { return mEbo.GetWritePtr(); }

        int UnmapVbo() const { return mVbo.Unmap(); }
        int UnmapEbo() const { return mEbo.Unmap(); }

        void BindVbo() const { mVbo.Bind(); }
        void UnBindVbo() const { mVbo.UnBind(); }

        void BindEbo() const { mEbo.Bind(); }
        void UnBindEbo() const { mEbo.UnBind(); }

        void BindMatrixBuffer() const { mModelMatrixBuffer->Bind(); }
        void UnBindMatrixBuffer() const { mModelMatrixBuffer->UnBind(); }

        int GetNumInstances() const { return mNumInstances; }

        void SetInstanceMatrix(const glm::mat4& modelMatrix, int instance = 0)
        { mModelMatrixBuffer->Modify(instance * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(modelMatrix)); }

        void CreateInstances(std::initializer_list<glm::mat4> modelMatrices);
        void CreateInstances(const std::vector<glm::mat4>& modelMatrices);
        void CreateInstances(int numInstances);

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(Func func)
        {
            for (int i = 0; i < mNumInstances; ++i)
                mModelMatrixBuffer->Modify(i * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(func(i, mNumInstances)));
        }

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(int numXMeshes, int numZMeshes, float xOffset, float zOffset, float yPos, Func func)
        {
            assert(numXMeshes * numZMeshes == mNumInstances);
            const float negNumXMeshesHalf = (xOffset) * static_cast<float>(-numXMeshes) * 0.5f;
            const float negNumZMeshesHalf = (zOffset) * static_cast<float>(-numZMeshes) * 0.5f;
            float* modelMatrixPtr = mModelMatrixBuffer->GetWritePtr();
            int cnt{};
            for (int i = 0; i < numXMeshes; ++i) {
                float xPos = negNumXMeshesHalf + static_cast<float>(i) * xOffset;
                for (int j = 0; j < numZMeshes; ++j) {
                    float zPos = negNumZMeshesHalf + static_cast<float>(j) * zOffset;
                    auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, zPos)) * func(i, j, mNumInstances);
                    std::memcpy(modelMatrixPtr + cnt * sizeof(glm::vec4), glm::value_ptr(transform), sizeof(glm::mat4));
                    ++cnt;
                }
            }
#ifdef _DEBUG
            assert(GL_TRUE == mModelMatrixBuffer->Unmap());
#else
            mModelMatrixBuffer->Unmap();
#endif
        }

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(int numXMeshes, int numYMeshes, int numZMeshes, float xOffset, float yOffset, float zOffset, Func func)
        {
            assert(numXMeshes * numYMeshes * numZMeshes == mNumInstances);
            const float negNumXMeshesHalf = (xOffset) * static_cast<float>(-numXMeshes) * 0.5f;
            const float negNumYMeshesHalf = (yOffset) * static_cast<float>(-numYMeshes) * 0.5f;
            const float negNumZMeshesHalf = (zOffset) * static_cast<float>(-numZMeshes) * 0.5f;
            float* modelMatrixPtr = mModelMatrixBuffer->GetWritePtr();
            int cnt{};
            for (int i = 0; i < numXMeshes; ++i) {
                float xPos = negNumXMeshesHalf + static_cast<float>(i) * xOffset;
                for (int j = 0; j < numYMeshes; ++j) {
                    float yPos = negNumYMeshesHalf + static_cast<float>(j) * yOffset;
                    for (int k = 0; k < numZMeshes; ++k) {
                        float zPos = negNumZMeshesHalf + static_cast<float>(k) * zOffset;
                        auto defaultTransform = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, zPos));
                        auto res = defaultTransform * func(i, j, k, mNumInstances);
                        std::memcpy(modelMatrixPtr + cnt * 16, glm::value_ptr(res), 64);
                        ++cnt;
                    }
                }
            }
#ifdef _DEBUG
            assert(GL_TRUE == mModelMatrixBuffer->Unmap());
#else
            mModelMatrixBuffer->Unmap();
#endif
        }
    };

    ////////////////////////////////////////
    StaticMesh CreateColoredTriangle(int numInstances);
    StaticMesh CreateColoredQuad(int numInstances);
    StaticMesh CreateColoredCircle(float radius, int numSegments, int numInstances);

    ////////////////////////////////////////
    StaticMesh CreateTriangle(int numInstances);
    StaticMesh CreateQuad(int numInstances);
    StaticMesh CreateCircle(float radius, int numSegments, int numInstances);
    StaticMesh CreateCube(int numInstances);
    StaticMesh CreateGrid(int numX, int numZ, int numInstances);
    StaticMesh CreatePyramid(int numInstances);
    StaticMesh CreateUVSphere(int numStacks, int numSectors, int numInstances);
    StaticMesh CreateIcoSphere(int numSubdivisions, int numInstances);

    ////////////////////////////////////////
    struct StaticModel
    {
    private:
        std::string mPath;
        std::string mDirectory;
        std::vector<StaticMesh> mMeshes;
        Texture2DLoader& mTexture2DLoader;
        int mNumTextures;
        int mNumInstances;

        void Load();
        void LoadNode(aiNode* node, const aiScene* scene);
        StaticMesh LoadStaticMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<std::reference_wrapper<const Texture2D>> Load2DTextures(aiMaterial* material, aiTextureType type, std::string_view typeName);

    public:
        StaticModel(const std::string& modelPath, Texture2DLoader& texture2DLoader)
            : mPath{modelPath},
              mTexture2DLoader{texture2DLoader},
              mNumTextures{},
              mNumInstances{} { Load(); }

        StaticModel(int numInstances, const std::string& modelPath, Texture2DLoader& texture2DLoader)
            : mPath{modelPath},
              mTexture2DLoader{texture2DLoader},
              mNumTextures{},
              mNumInstances{numInstances} { Load(); }

        void Draw(int mode = GL_TRIANGLES) const
        {
            for (const StaticMesh& staticMesh : mMeshes) {
                staticMesh.Bind();
                staticMesh.BindTextures();
                staticMesh.Draw(mode);
            }
        }

        void DrawInstanced(int mode = GL_TRIANGLES) const
        {
            for (const StaticMesh& staticMesh : mMeshes) {
                staticMesh.Bind();
                staticMesh.BindTextures();
                staticMesh.DrawInstanced(mode);
            }
        }

        std::string GetPath() const { return mPath; }
        std::string GetDirectory() const { return mDirectory; }
        int GetNumTextures() const { return mNumTextures; }
        int GetNumInstances() const { return mNumInstances; }

        void SetInstanceMatrix(const glm::mat4& modelMatrix, int instance = 0)
        { std::ranges::for_each(mMeshes, [&](auto& m){ m.SetInstanceMatrix(modelMatrix, instance); }); }

        void CreateInstances(std::initializer_list<glm::mat4> modelMatrices);
        void CreateInstances(const std::vector<glm::mat4>& modelMatrices);
        void CreateInstances(int numInstances);

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(Func func)
        { std::ranges::for_each(mMeshes, [&](auto& m){ m.ApplyToAllInstances(func); }); }

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(int numXMeshes, int numZMeshes, float xOffset, float zOffset, float yPos, Func func)
        { std::ranges::for_each(mMeshes, [&](auto& m){ m.ApplyToAllInstances(numXMeshes, numZMeshes, xOffset, zOffset, yPos, func); }); }

        ////////////////////////////////////////
        template <typename Func>
        void ApplyToAllInstances(int numXMeshes, int numYMeshes, int numZMeshes, float xOffset, float yOffset, float zOffset, Func func)
        { std::ranges::for_each(mMeshes, [&](auto& m){ m.ApplyToAllInstances(numXMeshes, numYMeshes, numZMeshes, xOffset, yOffset, zOffset, func); }); }
    };

    ////////////////////////////////////////
    StaticModel LoadCsItaly(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadDeDust(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadBackpack(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadViceCity(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadGTA2Downtown(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadGTA2Industrial(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadGTA2Residential(const std::string& rootPath, Texture2DLoader&);

    ////////////////////////////////////////
    struct PostProcessStack
    {
    private:
        int mWidth;
        int mHeight;
        int mNumSamples;

        PostProcessProgram mProgram;

        RenderbufferMultiSample mRboMS;
        Texture2DMultiSample mColor0MS;
        Framebuffer mFboMS;

        Texture2D mColor0;
        Framebuffer mFbo;

    public:
        PostProcessStack(const std::string& shaderRootPath, int width, int height, int numSamples, ShaderLoader&);

        void FirstPass() const { mFboMS.Bind(); }
        void SecondPass() const { mFboMS.Blit(mFbo, mWidth, mHeight); mFbo.UnBind(); }
        void BindColor0() const { mColor0.Bind(); }

        PostProcessProgram& Program() { return mProgram; }
        const PostProcessProgram& Program() const { return mProgram; }
    };

    ////////////////////////////////////////
    struct EmissiveColorMaterial
    {
        glm::vec4 mColor;
    };

    ////////////////////////////////////////
    struct AbstractEmissiveColorProgram
    {
    protected:
        Program mProgram;

    public:
        AbstractEmissiveColorProgram(const std::string& rootPath, ShaderLoader&, const std::string& vshaderUrl);

        virtual ~AbstractEmissiveColorProgram() {}

        static inline constexpr int COLOR_LOC = 0;
        static inline constexpr int MODEL_LOC = 1;

        void SetMaterial(const EmissiveColorMaterial& m) const
        { glUniform4fv(COLOR_LOC, 1, glm::value_ptr(m.mColor)); }

        virtual void SetModelMatrix(const glm::mat4& model) const = 0;

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }
    };

    ////////////////////////////////////////
    struct EmissiveColorProgramInstanced : public AbstractEmissiveColorProgram
    {
        EmissiveColorProgramInstanced(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& model) const override
        { glUniformMatrix4fv(MODEL_LOC, 1, GL_FALSE, glm::value_ptr(model)); }
    };

    ////////////////////////////////////////
    struct EmissiveColorProgram : public AbstractEmissiveColorProgram
    {
        EmissiveColorProgram(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& model) const override { }
    };

    ////////////////////////////////////////
    struct EmissiveTextureMaterial
    {
        glm::vec2 mTileMultiplier;
        glm::vec2 mTileOffset;
    };

    ////////////////////////////////////////
    struct EmissiveTextureProgramInstanced
    {
    private:
        Program mProgram;

    public:
        EmissiveTextureProgramInstanced(const std::string& rootPath, ShaderLoader&);

        static inline constexpr int EMISSIVE_TEXTURE_LOC = 0;
        static inline constexpr int TILE_MULTIPLIER_LOC = 1;
        static inline constexpr int TILE_OFFSET_LOC = 2;

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }

        void SetMaterial(const EmissiveTextureMaterial& m) const
        { glUniform2fv(TILE_MULTIPLIER_LOC, 1, glm::value_ptr(m.mTileMultiplier));
          glUniform2fv(TILE_OFFSET_LOC, 1, glm::value_ptr(m.mTileOffset)); }
    };

    ////////////////////////////////////////
    struct EmissiveTextureProgram
    {
    private:
        Program mProgram;

    public:
        EmissiveTextureProgram(const std::string& rootPath, ShaderLoader&);

        static inline constexpr int EMISSIVE_TEXTURE_LOC = 0;
        static inline constexpr int TILE_MULTIPLIER_LOC = 1;
        static inline constexpr int TILE_OFFSET_LOC = 2;
        static inline constexpr int MODEL_LOC = 3;

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }

        void SetMaterial(const EmissiveTextureMaterial& m) const
        { glUniform2fv(TILE_MULTIPLIER_LOC, 1, glm::value_ptr(m.mTileMultiplier));
          glUniform2fv(TILE_OFFSET_LOC, 1, glm::value_ptr(m.mTileOffset)); }

        void SetModelMatrix(const glm::mat4& model) const
        { glUniformMatrix4fv(MODEL_LOC, 1, GL_FALSE, glm::value_ptr(model)); }
    };

    ////////////////////////////////////////
    enum class DefaultSkyboxTexture{ UlukaiCorona, UlukaiRedEclipse, Cloudy };

    ////////////////////////////////////////
    struct TexturedSkyboxProgram
    {
    private:
        Program mProgram;
        Cubemap mCubemap;

        void Init();
        Cubemap ChooseCubemap(DefaultSkyboxTexture, const std::string&);

    public:
        TexturedSkyboxProgram(const std::string& rootPath, ShaderLoader&, std::initializer_list<std::pair<CubemapFace, std::string_view>> faces);
        TexturedSkyboxProgram(const std::string& rootPath, ShaderLoader&, DefaultSkyboxTexture);

        static inline constexpr int SKYBOX_LOC = 0;

        const Cubemap& GetCubemap() const { return mCubemap; }

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }

        void Draw() const
        { mProgram.Use(); mCubemap.Bind(); glDrawArrays(GL_TRIANGLES, 0, 36); }
    };

    ////////////////////////////////////////
    struct PbrLightProgramInstanced
    {
    private:
        Program mProgram;

    public:
        PbrLightProgramInstanced(const std::string& rootPath, ShaderLoader&);

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }
    };

    ////////////////////////////////////////
    struct PbrLightProgram
    {
    private:
        Program mProgram;

    public:
        PbrLightProgram(const std::string& rootPath, ShaderLoader&);

        static inline constexpr int MODEL_LOC = 0;

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }

        void SetModelMatrix(const glm::mat4& model) const
        { glUniformMatrix4fv(MODEL_LOC, 1, GL_FALSE, glm::value_ptr(model)); }
    };
}
