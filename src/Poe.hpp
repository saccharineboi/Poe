// Poe: OpenGL Renderer
// Copyright (C) 2023 saccharineboi
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
#include "Suppress.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

SUPPRESS_WARNINGS()
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
ENABLE_WARNINGS()

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
        unsigned mMode;
        size_t mNumElements;

    public:
        VertexBuffer(size_t numElements, unsigned mode);
        VertexBuffer(const std::vector<float>& vertices, unsigned mode);

        ~VertexBuffer() { glDeleteBuffers(1, &mId); }

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&&);
        VertexBuffer& operator=(VertexBuffer&&);

        void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

        unsigned GetId() const { return mId; }
        unsigned GetMode() const { return mMode; }
        size_t GetNumElements() const { return mNumElements; }

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
        unsigned mMode;
        size_t mNumElements;

    public:
        IndexBuffer(size_t numElements, unsigned mode);
        IndexBuffer(const std::vector<unsigned>& indices, unsigned mode);

        ~IndexBuffer() { glDeleteBuffers(1, &mId); }

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&&);
        IndexBuffer& operator=(IndexBuffer&&);

        void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

        unsigned GetId() const { return mId; }
        unsigned GetMode() const { return mMode; }
        size_t GetNumElements() const { return mNumElements; }

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
        size_t mSize;
        unsigned mMode;
        unsigned mBindLoc;

    public:
        static inline constexpr int FOG_BLOCK_BINDING{ 0 };
        static inline constexpr int TRANSFORM_BLOCK_BINDING{ 1 };
        static inline constexpr int PBR_LIGHT_MATERIAL_BLOCK_BINDING{ 2 };
        static inline constexpr int DIR_LIGHT_BLOCK_BINDING{ 3 };
        static inline constexpr int POSTPROCESS_BLOCK_BINDING{ 4 };
        static inline constexpr int BLINN_PHONG_MATERIAL_BLOCK_BINDING{ 5 };
        static inline constexpr int POINT_LIGHT_BLOCK_BINDING{ 6 };
        static inline constexpr int SPOT_LIGHT_BLOCK_BINDING{ 7 };
        static inline constexpr int REALISTIC_SKYBOX_BLOCK_BINDING{ 8 };

        UniformBuffer(size_t size, unsigned mode, unsigned bindLoc);

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
        size_t GetSize() const { return mSize; }
        unsigned GetMode() const { return mMode; }
        unsigned GetBindLoc() const { return mBindLoc; }
    };

    ////////////////////////////////////////
    struct RealisticSkyboxMaterial
    {
        glm::vec3 mRayleighScatteringCoefficient;
        glm::vec3 mRayOrigin;
        glm::vec3 mSunPosition;
        float mSunIntensity;
        float mPlanetRadius;
        float mAtmosphereRadius;
        float mMieScatteringCoefficient;
        float mRayleighScaleHeight;
        float mMieScaleHeight;
        float mMiePreferredScatteringDirection;
    };

    ////////////////////////////////////////
    struct RealisticSkyboxUB__DATA
    {
        alignas(16) float rayleighScatteringCoefficient[3];
        alignas(16) float rayOrigin[3];
        alignas(16) float sunPosition[3];
        alignas(16) float sunIntensity;
        alignas(4) float planetRadius;
        alignas(4) float atmosphereRadius;
        alignas(4) float mieScatteringCoefficient;
        alignas(4) float rayleighScaleHeight;
        alignas(4) float mieScaleHeight;
        alignas(4) float miePreferredScatteringDirection;

        void SetRayleighScatteringCoefficient(const glm::vec3& v)
        { std::memcpy(rayleighScatteringCoefficient, glm::value_ptr(v), sizeof(glm::vec3)); }

        void SetRayOrigin(const glm::vec3& v)
        { std::memcpy(rayOrigin, glm::value_ptr(v), sizeof(glm::vec3)); }

        void SetSunPosition(const glm::vec3& v)
        { std::memcpy(sunPosition, glm::value_ptr(v), sizeof(glm::vec3)); }

        void SetSunIntensity(float intensity)
        { sunIntensity = intensity; }

        void SetPlanetRadius(float radius)
        { planetRadius = radius; }

        void SetAtmosphereRadius(float radius)
        { atmosphereRadius = radius; }

        void SetMieScatteringCoefficient(float coefficient)
        { mieScatteringCoefficient = coefficient; }

        void SetRayleighScaleHeight(float height)
        { rayleighScaleHeight = height; }

        void SetMieScaleHeight(float height)
        { mieScaleHeight = height; }

        void SetMiePreferredScatteringDirection(float d)
        { miePreferredScatteringDirection = d; }

        void Set(const RealisticSkyboxMaterial& material)
        {
            SetRayleighScatteringCoefficient(material.mRayleighScatteringCoefficient);
            SetRayOrigin(material.mRayOrigin);
            SetSunPosition(material.mSunPosition);
            SetSunIntensity(material.mSunIntensity);
            SetPlanetRadius(material.mPlanetRadius);
            SetAtmosphereRadius(material.mAtmosphereRadius);
            SetMieScatteringCoefficient(material.mMieScatteringCoefficient);
            SetRayleighScaleHeight(material.mRayleighScaleHeight);
            SetMiePreferredScatteringDirection(material.mMiePreferredScatteringDirection);
        }

        glm::vec3 GetRayleighScatteringCoefficient() const
        { return glm::vec3(rayleighScatteringCoefficient[0], rayleighScatteringCoefficient[1], rayleighScatteringCoefficient[2]); }

        glm::vec3 GetRayOrigin() const
        { return glm::vec3(rayOrigin[0], rayOrigin[1], rayOrigin[2]); }

        glm::vec3 GetSunPosition() const
        { return glm::vec3(sunPosition[0], sunPosition[1], sunPosition[2]); }

        float GetSunIntensity() const { return sunIntensity; }
        float GetPlanetRadius() const { return planetRadius; }
        float GetAtmosphereRadius() const { return atmosphereRadius; }
        float GetMieScatteringCoefficient() const { return mieScatteringCoefficient; }
        float GetRayleighScaleHeight() const { return rayleighScaleHeight; }
        float GetMieScaleHeight() const { return mieScaleHeight; }
        float GetMiePreferredScatteringDirection() const { return miePreferredScatteringDirection; }

        RealisticSkyboxMaterial Get() const
        {
            return RealisticSkyboxMaterial{ GetRayleighScatteringCoefficient(),
                                            GetRayOrigin(),
                                            GetSunPosition(),
                                            GetSunIntensity(),
                                            GetPlanetRadius(),
                                            GetAtmosphereRadius(),
                                            GetMieScatteringCoefficient(),
                                            GetRayleighScaleHeight(),
                                            GetMieScaleHeight(),
                                            GetMiePreferredScatteringDirection() };
        }
    };

    ////////////////////////////////////////
    struct RealisticSkyboxUB
    {
    private:
        UniformBuffer mBuffer;
        RealisticSkyboxUB__DATA mData;

    public:
        RealisticSkyboxUB(); // default is earth atmosphere

        const UniformBuffer& Buffer() const { return mBuffer; }

        void SetRayleighScatteringCoefficient(const glm::vec3& v)
        { mData.SetRayleighScatteringCoefficient(v); }

        void SetRayOrigin(const glm::vec3& v)
        { mData.SetRayOrigin(v); }

        void SetSunPosition(const glm::vec3& v)
        { mData.SetSunPosition(v); }

        void SetSunIntensity(float intensity)
        { mData.SetSunIntensity(intensity); }

        void SetPlanetRadius(float radius)
        { mData.SetPlanetRadius(radius); }

        void SetAtmosphereRadius(float radius)
        { mData.SetAtmosphereRadius(radius); }

        void SetMieScatteringCoefficient(float coefficient)
        { mData.SetMieScatteringCoefficient(coefficient); }

        void SetRayleighScaleHeight(float height)
        { mData.SetRayleighScaleHeight(height); }

        void SetMieScaleHeight(float height)
        { mData.SetMieScaleHeight(height); }

        void SetMiePreferredScatteringDirection(float d)
        { mData.SetMiePreferredScatteringDirection(d); }

        void Set(const RealisticSkyboxMaterial& material)
        { mData.Set(material); }

        glm::vec3 GetRayleighScatteringCoefficient() const
        { return mData.GetRayleighScatteringCoefficient(); }

        glm::vec3 GetRayOrigin() const
        { return mData.GetRayOrigin(); }

        glm::vec3 GetSunPosition() const
        { return mData.GetSunPosition(); }

        float GetSunIntensity() const { return mData.GetSunIntensity(); }
        float GetPlanetRadius() const { return mData.GetPlanetRadius(); }
        float GetAtmosphereRadius() const { return mData.GetAtmosphereRadius(); }
        float GetMieScatteringCoefficient() const { return mData.GetMieScatteringCoefficient(); }
        float GetRayleighScaleHeight() const { return mData.GetRayleighScaleHeight(); }
        float GetMieScaleHeight() const { return mData.GetMieScaleHeight(); }
        float GetMiePreferredScatteringDirection() const { return mData.GetMiePreferredScatteringDirection(); }

        RealisticSkyboxMaterial Get() const { return mData.Get(); }

        void Update() const { mBuffer.Modify(0, sizeof(RealisticSkyboxUB__DATA), &mData); }
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
    struct PostProcessUB__DATA
    {
        alignas(4) float grayscaleWeight;
        alignas(4) float kernelWeight;
        alignas(4) float gamma;
        alignas(4) float exposure;
        alignas(16) float kernel[9];
    };

    ////////////////////////////////////////
    struct PostProcessUB
    {
    private:
        UniformBuffer mBuffer;
        PostProcessUB__DATA mData;

    public:
        PostProcessUB();

        const UniformBuffer& Buffer() const { return mBuffer; }

        float GetGrayscaleWeight() const { return mData.grayscaleWeight; }
        float GetKernelWeight() const { return mData.kernelWeight; }
        float GetGamma() const { return mData.gamma; }
        float GetExposure() const { return mData.exposure; }
        glm::mat3 GetKernel() const { return glm::mat3(mData.kernel[0], mData.kernel[1], mData.kernel[2],
                                                       mData.kernel[3], mData.kernel[4], mData.kernel[5],
                                                       mData.kernel[6], mData.kernel[7], mData.kernel[8]); }

        void SetGrayscaleWeight(float weight) { mData.grayscaleWeight = weight; }
        void SetKernelWeight(float weight) { mData.kernelWeight = weight; }
        void SetGamma(float gamma) { mData.gamma = gamma; }
        void SetExposure(float exposure) { mData.exposure = exposure; }
        void SetKernel(const glm::mat3& kernel) { std::memcpy(mData.kernel, glm::value_ptr(kernel), sizeof(glm::mat3)); }

        void SetIdentityKernel()
        {
            glm::mat3 kernel(0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f);
            std::memcpy(mData.kernel, glm::value_ptr(kernel), sizeof(glm::mat3));
        }

        void SetSharpenKernel()
        {
            glm::mat3 kernel(2.0f,  2.0f,  2.0f,
                             2.0f, -15.0f, 2.0f,
                             2.0f,  2.0f,  2.0f);
            std::memcpy(mData.kernel, glm::value_ptr(kernel), sizeof(glm::mat3));
        }

        void SetBlurKernel()
        {
            glm::mat3 kernel(1.0f, 2.0f, 1.0f,
                             2.0f, 4.0f, 2.0f,
                             1.0f, 2.0f, 1.0f);
            std::memcpy(mData.kernel, glm::value_ptr(kernel / 16.0f), sizeof(glm::mat3));
        }

        void SetEdgeDetectKernel()
        {
            glm::mat3 kernel(1.0f,  1.0f, 1.0f,
                             1.0f, -8.0f, 1.0f,
                             1.0f,  1.0f, 1.0f);
            std::memcpy(mData.kernel, glm::value_ptr(kernel / 16.0f), sizeof(glm::mat3));
        }

        void SetEmbossKernel()
        {
            glm::mat3 kernel(-2.0f, -1.0f, 0.0f,
                             -1.0f,  1.0f, 1.0f,
                              0.0f,  1.0f, 2.0f);
            std::memcpy(mData.kernel, glm::value_ptr(kernel / 16.0f), sizeof(glm::mat3));
        }

        void Update() const { mBuffer.Modify(0, sizeof(PostProcessUB__DATA), &mData); }
    };

    ////////////////////////////////////////
    struct FirstPersonCamera;

    ////////////////////////////////////////
    struct TransformUB__DATA
    {
        alignas(16) float projection_data[16];
        alignas(16) float view_data[16];
        alignas(16) float projView_data[16];
        alignas(16) float camDir_data[3];

        void SetProjectionData(const glm::mat4& projectionMatrix)
        { std::memcpy(projection_data, glm::value_ptr(projectionMatrix), sizeof(glm::mat4)); }

        void SetViewData(const glm::mat4& viewMatrix)
        { std::memcpy(view_data, glm::value_ptr(viewMatrix), sizeof(glm::mat4)); }

        void SetProjViewData(const glm::mat4& projViewMatrix)
        { std::memcpy(projView_data, glm::value_ptr(projViewMatrix), sizeof(glm::mat4)); }

        void SetCamDirData(const glm::vec3& camDir)
        { std::memcpy(camDir_data, glm::value_ptr(camDir), sizeof(glm::vec3)); }
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
    struct BlinnPhongMaterial
    {
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        float mShininess;
    };

    ////////////////////////////////////////
    struct BlinnPhongMaterial__DATA
    {
        alignas(16) float ambient[3];
        alignas(16) float diffuse[3];
        alignas(16) float specular[3];
        alignas(16) float shininess;

        void SetAmbient(const glm::vec3& ambientColor)
        { std::memcpy(ambient, glm::value_ptr(ambientColor), sizeof(glm::vec3)); }

        void SetDiffuse(const glm::vec3& diffuseColor)
        { std::memcpy(diffuse, glm::value_ptr(diffuseColor), sizeof(glm::vec3)); }

        void SetSpecular(const glm::vec3& specularColor)
        { std::memcpy(specular, glm::value_ptr(specularColor), sizeof(glm::vec3)); }

        void SetShininess(float s)
        { shininess = s; }

        void Set(const BlinnPhongMaterial& material)
        {
            SetAmbient(material.mAmbient);
            SetDiffuse(material.mDiffuse);
            SetSpecular(material.mSpecular);
            SetShininess(material.mShininess);
        }

        glm::vec3 GetAmbient() const
        { return glm::vec3(ambient[0], ambient[1], ambient[2]); }

        glm::vec3 GetDiffuse() const
        { return glm::vec3(diffuse[0], diffuse[1], diffuse[2]); }

        glm::vec3 GetSpecular() const
        { return glm::vec3(specular[0], specular[1], specular[2]); }

        float GetShininess() const { return shininess; }

        BlinnPhongMaterial Get() const
        {
            return BlinnPhongMaterial{ GetAmbient(),
                                       GetDiffuse(),
                                       GetSpecular(),
                                       GetShininess() };
        }
    };

    ////////////////////////////////////////
    struct BlinnPhongMaterialUB
    {
    private:
        UniformBuffer mBuffer;
        BlinnPhongMaterial__DATA mData;

    public:
        BlinnPhongMaterialUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        void Update() const
        { mBuffer.Modify(0, sizeof(BlinnPhongMaterial__DATA), &mData); }

        void SetAmbient(const glm::vec3& ambient)
        { mData.SetAmbient(ambient); }

        void SetDiffuse(const glm::vec3& diffuse)
        { mData.SetDiffuse(diffuse); }

        void SetSpecular(const glm::vec3& specular)
        { mData.SetSpecular(specular); }

        void SetShininess(float shininess)
        { mData.SetShininess(shininess); }

        void Set(const BlinnPhongMaterial& material)
        { mData.Set(material); }

        glm::vec3 GetAmbient() const
        { return mData.GetAmbient(); }

        glm::vec3 GetDiffuse() const
        { return mData.GetDiffuse(); }

        glm::vec3 GetSpecular() const
        { return mData.GetSpecular(); }

        float GetShininess() const
        { return mData.GetShininess(); }

        BlinnPhongMaterial Get() const
        { return mData.Get(); }
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
        alignas(16) float metallic;
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
    struct PointLight
    {
        glm::vec3 mColor;
        glm::vec3 mDirection;
        glm::vec3 mPosition;
        float mRadius;
        float mIntensity;
    };

    ////////////////////////////////////////
    struct SpotLight
    {
        glm::vec3 mColor;
        glm::vec3 mDirection;
        glm::vec3 mPosition;
        float mInnerCutoff;
        float mOuterCutoff;
        float mRadius;
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
    struct PointLight__DATA
    {
        alignas(16) float color[3];
        alignas(16) float direction[3];
        alignas(16) float position[3];
        alignas(4) float constant;
        alignas(4) float linear;
        alignas(4) float quadratic;
        alignas(4) float intensity;
    };

    ////////////////////////////////////////
    struct SpotLight__DATA
    {
        alignas(16) float color[3];
        alignas(16) float direction[3];
        alignas(16) float position[3];
        alignas(4) float innerCutoff;
        alignas(4) float outerCutoff;
        alignas(4) float constant;
        alignas(4) float linear;
        alignas(4) float quadratic;
        alignas(4) float intensity;
    };

    ////////////////////////////////////////
    inline constexpr int NUM_DIR_LIGHTS{ 2 };
    inline constexpr int NUM_POINT_LIGHTS{ 4 };
    inline constexpr int NUM_SPOT_LIGHTS{ 4 };

    ////////////////////////////////////////
    struct alignas(16) DirLightListElem__DATA
    {
        DirLight__DATA data;

        void SetColor(const glm::vec3& color)
        { std::memcpy(data.color, glm::value_ptr(color), sizeof(glm::vec3)); }

        void SetDirection(const glm::mat4& viewMatrix, const glm::vec3& dir)
        { std::memcpy(data.direction, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(dir, 0.0f))), sizeof(glm::vec3)); }

        void SetIntensity(float intensity)
        { data.intensity = intensity; }

        glm::vec3 GetColor() const
        { return glm::vec3(data.color[0], data.color[1], data.color[2]); }

        glm::vec3 GetDirection() const
        { return glm::vec3(data.direction[0], data.direction[1], data.direction[2]); }

        float GetIntensity() const { return data.intensity; }
    };

    ////////////////////////////////////////
    struct alignas(16) PointLightListElem__DATA
    {
        PointLight__DATA data;

        void SetColor(const glm::vec3& color)
        { std::memcpy(data.color, glm::value_ptr(color), sizeof(glm::vec3)); }

        void SetDirection(const glm::mat4& viewMatrix, const glm::vec3& dir)
        { std::memcpy(data.direction, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(dir, 0.0f))), sizeof(glm::vec3)); }

        void SetPosition(const glm::mat4& viewMatrix, const glm::vec3& pos)
        { std::memcpy(data.position, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pos, 1.0f))), sizeof(glm::vec3)); }

        void SetConstant(float constant) { data.constant = constant; }
        void SetLinear(float linear) { data.linear = linear; }
        void SetQuadratic(float quadratic) { data.quadratic = quadratic;  }

        void SetRadius(float radius)
        {
            data.constant = 1.0f;
            data.linear = 4.5f / radius;
            data.quadratic = 75.0f / (radius * radius);
        }

        void SetIntensity(float intensity) { data.intensity = intensity; }

        glm::vec3 GetColor() const
        { return glm::vec3(data.color[0], data.color[1], data.color[2]); }

        glm::vec3 GetDirection() const
        { return glm::vec3(data.direction[0], data.direction[1], data.direction[2]); }

        glm::vec3 GetPosition() const
        { return glm::vec3(data.position[0], data.position[1], data.position[2]); }

        float GetConstant() const { return data.constant; }
        float GetLinear() const { return data.linear; }
        float GetQuadratic() const { return data.quadratic; }

        float GetRadius() const { return 4.5f / data.linear; }

        float GetIntensity() const { return data.intensity; }
    };

    ////////////////////////////////////////
    struct alignas(16) SpotLightListElem__DATA
    {
        SpotLight__DATA data;

        void SetColor(const glm::vec3& color)
        { std::memcpy(data.color, glm::value_ptr(color), sizeof(glm::vec3)); }

        void SetDirection(const glm::mat4& viewMatrix, const glm::vec3& dir)
        { std::memcpy(data.direction, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(dir, 0.0f))), sizeof(glm::vec3)); }

        void SetPosition(const glm::mat4& viewMatrix, const glm::vec3& pos)
        { std::memcpy(data.position, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pos, 1.0f))), sizeof(glm::vec3)); }

        void SetInnerCutoff(float innerCutoff) { data.innerCutoff = innerCutoff; }
        void SetOuterCutoff(float outerCutoff) { data.outerCutoff = outerCutoff; }

        void SetConstant(float constant) { data.constant = constant; }
        void SetLinear(float linear) { data.linear = linear; }
        void SetQuadratic(float quadratic) { data.quadratic = quadratic;  }

        void SetRadius(float radius)
        {
            data.constant = 1.0f;
            data.linear = 4.5f / radius;
            data.quadratic = 75.0f / (radius * radius);
        }

        void SetIntensity(float intensity) { data.intensity = intensity; }

        glm::vec3 GetColor() const
        { return glm::vec3(data.color[0], data.color[1], data.color[2]); }

        glm::vec3 GetDirection() const
        { return glm::vec3(data.direction[0], data.direction[1], data.direction[2]); }

        glm::vec3 GetPosition() const
        { return glm::vec3(data.position[0], data.position[1], data.position[2]); }

        float GetConstant() const { return data.constant; }
        float GetLinear() const { return data.linear; }
        float GetQuadratic() const { return data.quadratic; }

        float GetRadius() const { return 4.5f / data.linear; }

        float GetInnerCutoff() const { return data.innerCutoff; }
        float GetOuterCutoff() const { return data.outerCutoff; }

        float GetIntensity() const { return data.intensity; }
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
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            mLightsData[ind].SetColor(color);
        }

        void SetDirection(int ind, const glm::mat4& viewMatrix, const glm::vec3& dir)
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            mLightsData[ind].SetDirection(viewMatrix, dir);
        }

        void SetIntensity(int ind, float intensity)
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            mLightsData[ind].SetIntensity(intensity);
        }

        void Update() const
        { mBuffer.Modify(0, DATA_SIZE, mLightsData); }

        void Set(int ind, const glm::mat4& viewMatrix, const DirLight& dirLight)
        {
            SetColor(ind, dirLight.mColor);
            SetDirection(ind, viewMatrix, dirLight.mDirection);
            SetIntensity(ind, dirLight.mIntensity);
        }

        glm::vec3 GetColor(int ind) const
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            return mLightsData[ind].GetColor();
        }

        glm::vec3 GetDirection(int ind) const
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            return mLightsData[ind].GetDirection();
        }

        float GetIntensity(int ind) const
        {
            assert(ind >= 0 && ind < NUM_DIR_LIGHTS);
            return mLightsData[ind].GetIntensity();
        }

        DirLight Get(int ind) const
        {
            return DirLight{ GetColor(ind),
                             GetDirection(ind),
                             GetIntensity(ind) };
        }
    };

    ////////////////////////////////////////
    struct PointLightUB
    {
    private:
        UniformBuffer mBuffer;
        PointLightListElem__DATA mLightsData[NUM_POINT_LIGHTS];

    public:
        PointLightUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        inline static constexpr int DATA_SIZE = sizeof(PointLightListElem__DATA) * NUM_POINT_LIGHTS;

        void SetColor(int ind, const glm::vec3& color)
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            mLightsData[ind].SetColor(color);
        }

        void SetDirection(int ind, const glm::mat4& viewMatrix, const glm::vec3& dir)
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            mLightsData[ind].SetDirection(viewMatrix, dir);
        }

        void SetPosition(int ind, const glm::mat4& viewMatrix, const glm::vec3& pos)
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            mLightsData[ind].SetPosition(viewMatrix, pos);
        }

        void SetRadius(int ind, float radius)
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            mLightsData[ind].SetRadius(radius);
        }

        void SetIntensity(int ind, float intensity)
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            mLightsData[ind].SetIntensity(intensity);
        }

        void Set(int ind, const glm::mat4& viewMatrix, const PointLight& pl)
        {
            SetColor(ind, pl.mColor);
            SetDirection(ind, viewMatrix, pl.mDirection);
            SetPosition(ind, viewMatrix, pl.mPosition);
            SetRadius(ind, pl.mRadius);
            SetIntensity(ind, pl.mIntensity);
        }

        glm::vec3 GetColor(int ind) const
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            return mLightsData[ind].GetColor();
        }

        glm::vec3 GetDirection(int ind) const
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            return mLightsData[ind].GetDirection();
        }

        glm::vec3 GetPosition(int ind) const
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            return mLightsData[ind].GetPosition();
        }

        float GetRadius(int ind) const
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            return mLightsData[ind].GetRadius();
        }

        float GetIntensity(int ind) const
        {
            assert(ind >= 0 && ind < NUM_POINT_LIGHTS);
            return mLightsData[ind].GetIntensity();
        }

        PointLight Get(int ind) const
        {
            return PointLight{ GetColor(ind),
                               GetDirection(ind),
                               GetPosition(ind),
                               GetRadius(ind),
                               GetIntensity(ind) };
        }
    };

    ////////////////////////////////////////
    struct SpotLightUB
    {
    private:
        UniformBuffer mBuffer;
        SpotLightListElem__DATA mLightsData[NUM_SPOT_LIGHTS];

    public:
        SpotLightUB();
        const UniformBuffer& Buffer() const { return mBuffer; }

        inline static constexpr int DATA_SIZE = sizeof(SpotLightListElem__DATA) * NUM_SPOT_LIGHTS;

        void SetColor(int ind, const glm::vec3& color)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetColor(color);
        }

        void SetDirection(int ind, const glm::mat4& viewMatrix, const glm::vec3& dir)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetDirection(viewMatrix, dir);
        }

        void SetPosition(int ind, const glm::mat4& viewMatrix, const glm::vec3& pos)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetPosition(viewMatrix, pos);
        }

        void SetInnerCutoff(int ind, float innerCutoff)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetInnerCutoff(innerCutoff);
        }

        void SetOuterCutoff(int ind, float outerCutoff)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetOuterCutoff(outerCutoff);
        }

        void SetRadius(int ind, float radius)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetRadius(radius);
        }

        void SetIntensity(int ind, float intensity)
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            mLightsData[ind].SetIntensity(intensity);
        }

        void Set(int ind, const glm::mat4& viewMatrix, const SpotLight& sp)
        {
            SetColor(ind, sp.mColor);
            SetDirection(ind, viewMatrix, sp.mDirection);
            SetPosition(ind, viewMatrix, sp.mPosition);
            SetInnerCutoff(ind, sp.mInnerCutoff);
            SetOuterCutoff(ind, sp.mOuterCutoff);
            SetRadius(ind, sp.mRadius);
            SetIntensity(ind, sp.mIntensity);
        }

        glm::vec3 GetColor(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetColor();
        }

        glm::vec3 GetDirection(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetDirection();
        }

        glm::vec3 GetPosition(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetPosition();
        }

        float GetInnerCutoff(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetInnerCutoff();
        }

        float GetOuterCutoff(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetOuterCutoff();
        }

        float GetRadius(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetRadius();
        }

        float GetIntensity(int ind) const
        {
            assert(ind >= 0 && ind < NUM_SPOT_LIGHTS);
            return mLightsData[ind].GetIntensity();
        }

        SpotLight Get(int ind) const
        {
            return SpotLight{ GetColor(ind),
                              GetDirection(ind),
                              GetPosition(ind),
                              GetInnerCutoff(ind),
                              GetOuterCutoff(ind),
                              GetRadius(ind),
                              GetIntensity(ind) };
        }
    };

    ////////////////////////////////////////
    struct VertexInfo
    {
        unsigned loc;
        int numElements;
        unsigned dataType;
        int stride;
        unsigned offset;
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

        void Draw(unsigned mode = GL_TRIANGLES) const
        { glDrawElements(mode, mNumIndices, GL_UNSIGNED_INT, nullptr); }

        void DrawInstanced(unsigned mode = GL_TRIANGLES, int numInstances = 1) const
        { glDrawElementsInstanced(mode, mNumIndices, GL_UNSIGNED_INT, nullptr, numInstances); }

        unsigned GetId() const { return mId; }
        int GetNumIndices() const { return mNumIndices; }
    };

    ////////////////////////////////////////
    struct Shader
    {
    private:
        unsigned mId;
        unsigned mType;

    public:
        Shader(unsigned type, const std::string& source);

        ~Shader() { glDeleteShader(mId); }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&&);
        Shader& operator=(Shader&&);

        unsigned GetId() const { return mId; }
        unsigned GetType() const { return mType; }
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
                    // TODO handle the error
                }
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
        Shader& Load(unsigned type, std::string_view shaderUrl);
    };

    ////////////////////////////////////////
    Program CreateBasicProgram(const std::string& rootPath, ShaderLoader&);

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

        static inline constexpr int SCREEN_TEXTURE_LOC = 0;
        static inline constexpr int TEXELSTRETCH_LOC = 1;
    };

    ////////////////////////////////////////
    struct Texture2DParams
    {
        unsigned textureFormat = GL_RGB;
        unsigned internalFormat = GL_RGB8;
        bool generateMipmaps = true;
        float maxAnisotropy = 16.0f;
        int wrapS = GL_REPEAT;
        int wrapT = GL_REPEAT;
        int minF = GL_LINEAR_MIPMAP_LINEAR;
        int magF = GL_LINEAR;
        unsigned type = GL_UNSIGNED_BYTE;
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

        unsigned GetTextureFormat() const { return mParams.textureFormat; }
        unsigned GetInternalFormat() const { return mParams.internalFormat; }
        bool HasMipmaps() const { return mParams.generateMipmaps; }
        float GetMaxAnisotropy() const { return mParams.maxAnisotropy; }
        int GetWrapS() const { return mParams.wrapS; }
        int GetWrapT() const { return mParams.wrapT; }
        int GetMinF() const { return mParams.minF; }
        int GetMagF() const { return mParams.magF; }
        unsigned GetType() const { return mParams.type; }

        int GetNumMipmaps() const { return mNumMipmaps; }

        void Bind(unsigned loc = 0) const { glBindTextureUnit(loc, mId); }
        void UnBind(unsigned loc = 0) const { glBindTextureUnit(loc, 0); }
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
    struct Renderbuffer
    {
    private:
        unsigned mId;
        unsigned mType;
        int mWidth, mHeight;

    public:
        Renderbuffer(unsigned type, int width, int height);

        ~Renderbuffer() { glDeleteRenderbuffers(1, &mId); }

        Renderbuffer(const Renderbuffer&) = delete;
        Renderbuffer& operator=(const Renderbuffer&) = delete;

        Renderbuffer(Renderbuffer&&);
        Renderbuffer& operator=(Renderbuffer&&);

        void Bind() const { glBindRenderbuffer(GL_RENDERBUFFER, mId); }
        void UnBind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

        unsigned GetId() const { return mId; }
        unsigned GetType() const { return mType; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
    };

    ////////////////////////////////////////
    struct RenderbufferMultiSample
    {
    private:
        unsigned mId;
        unsigned mType;
        int mWidth;
        int mHeight;
        int mNumSamples;

    public:
        RenderbufferMultiSample(unsigned type, int width, int height, int numSamples);

        ~RenderbufferMultiSample() { glDeleteRenderbuffers(1, &mId); }

        RenderbufferMultiSample(const RenderbufferMultiSample&) = delete;
        RenderbufferMultiSample& operator=(const RenderbufferMultiSample&) = delete;

        RenderbufferMultiSample(RenderbufferMultiSample&&);
        RenderbufferMultiSample& operator=(RenderbufferMultiSample&&);

        unsigned GetId() const { return mId; }
        unsigned GetType() const { return mType; }
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
        unsigned mType;
        int mNumSamples;

    public:
        Texture2DMultiSample(int width, int height, unsigned type, int numSamples);

        ~Texture2DMultiSample() { glDeleteTextures(1, &mId); }

        Texture2DMultiSample(const Texture2DMultiSample&) = delete;
        Texture2DMultiSample& operator=(const Texture2DMultiSample&) = delete;

        Texture2DMultiSample(Texture2DMultiSample&&);
        Texture2DMultiSample& operator=(Texture2DMultiSample&&);

        unsigned GetId() const { return mId; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        unsigned GetType() const { return mType; }
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

        void Blit(const Framebuffer& fb, int width, int height, int outputWidth, int outputHeight) const
        { glBlitNamedFramebuffer(mId, fb.GetId(), 0, 0, width, height, 0, 0, outputWidth, outputHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST); }

        unsigned GetId() const { return mId; }
    };

    ////////////////////////////////////////
    struct StaticMesh
    {
    private:
        VertexBuffer mVbo;
        IndexBuffer mEbo;
        VAO mVao;
        std::unique_ptr<VertexBuffer> mModelMatrixBuffer;
        int mNumInstances;

        std::vector<std::reference_wrapper<const Texture2D>> mAmbientTextures;
        std::vector<std::reference_wrapper<const Texture2D>> mDiffuseTextures;
        std::vector<std::reference_wrapper<const Texture2D>> mSpecularTextures;

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
                   size_t numVertices,
                   size_t numIndices,
                   const std::vector<VertexInfo>& infos)
            : mVbo(numVertices, GL_STATIC_DRAW),
              mEbo(numIndices, GL_STATIC_DRAW),
              mVao(mVbo, mEbo, infos),
              mModelMatrixBuffer{new VertexBuffer(16, GL_DYNAMIC_DRAW)},
              mNumInstances{numInstances}
        { CreateInstances(mNumInstances); }

        void Bind() const { mVao.Bind(); }
        void UnBind() const { mVao.UnBind(); }

        void Draw(unsigned mode = GL_TRIANGLES) const
        { mVao.Draw(mode); }

        void DrawInstanced(unsigned mode = GL_TRIANGLES) const
        { mVao.DrawInstanced(mode, mNumInstances); }

        void AddAmbientTexture(const Texture2D& t)
        { mAmbientTextures.push_back(t); }

        void AddDiffuseTexture(const Texture2D& t)
        { mDiffuseTextures.push_back(t); }

        void AddSpecularTexture(const Texture2D& t)
        { mSpecularTextures.push_back(t); }

        void BindTextures() const
        {
            if (mAmbientTextures.size() > 0 && mDiffuseTextures.size() > 0 && mSpecularTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mAmbientTextures[0]).Bind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(1);
                static_cast<const Texture2D&>(mSpecularTextures[0]).Bind(2);
            }
            else if (mDiffuseTextures.size() > 0 && mSpecularTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(1);
                static_cast<const Texture2D&>(mSpecularTextures[0]).Bind(2);
            }
            else if (mDiffuseTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(1);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).Bind(2);
            }
        }

        void UnbindTextures() const
        {
            if (mAmbientTextures.size() > 0 && mDiffuseTextures.size() > 0 && mSpecularTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mAmbientTextures[0]).UnBind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(1);
                static_cast<const Texture2D&>(mSpecularTextures[0]).UnBind(2);
            }
            else if (mDiffuseTextures.size() > 0 && mSpecularTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(1);
                static_cast<const Texture2D&>(mSpecularTextures[0]).UnBind(2);
            }
            else if (mDiffuseTextures.size() > 0)
            {
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(0);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(1);
                static_cast<const Texture2D&>(mDiffuseTextures[0]).UnBind(2);
            }
        }

        size_t GetNumVertices() const { return mVbo.GetNumElements(); }
        size_t GetNumIndices() const { return mEbo.GetNumElements(); }

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
        { mModelMatrixBuffer->Modify(instance * static_cast<int>(sizeof(glm::mat4)), sizeof(glm::mat4), glm::value_ptr(modelMatrix)); }

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

        void Draw(unsigned mode = GL_TRIANGLES) const
        {
            for (const StaticMesh& staticMesh : mMeshes) {
                staticMesh.Bind();
                staticMesh.BindTextures();
                staticMesh.Draw(mode);
            }
        }

        void DrawInstanced(unsigned mode = GL_TRIANGLES) const
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
    StaticModel LoadSponza(const std::string& rootPath, Texture2DLoader&);
    StaticModel LoadDeDust(const std::string& rootPath, Texture2DLoader&);

    ////////////////////////////////////////
    struct PostProcessStack
    {
    private:
        int mWidth;
        int mHeight;
        int mOutputWidth;
        int mOutputHeight;
        int mNumSamples;

        PostProcessProgram mProgram;

        RenderbufferMultiSample mRboMS;
        Texture2DMultiSample mColor0MS;
        Framebuffer mFboMS;

        Texture2D mColor0;
        Framebuffer mFbo;

    public:
        PostProcessStack(const std::string& shaderRootPath,
                         int width, int height,
                         int numSamples, ShaderLoader&);

        PostProcessStack(const std::string& shaderRootPath,
                         int width, int height,
                         int outputWidth, int outputHeight,
                         int numSamples, ShaderLoader&);

        void FirstPass() const { glViewport(0, 0, mWidth, mHeight); mFboMS.Bind(); }
        void SecondPass() const { mFboMS.Blit(mFbo, mWidth, mHeight); mFbo.UnBind(); glViewport(0, 0, mOutputWidth, mOutputHeight); }
        void BindColor0() const { mColor0.Bind(); }

        PostProcessProgram& Program() { return mProgram; }
        const PostProcessProgram& Program() const { return mProgram; }

        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        int GetOutputWidth() const { return mOutputWidth; }
        int GetOutputHeight() const { return mOutputHeight; }
        int GetNumSamples() const { return mNumSamples; }

        void Use() const
        {
            mProgram.Use();

            float widthRatio{ static_cast<float>(mWidth) / static_cast<float>(mOutputWidth) };
            float heightRatio{ static_cast<float>(mHeight) / static_cast<float>(mOutputHeight) };
            glUniform2f(PostProcessProgram::TEXELSTRETCH_LOC, widthRatio, heightRatio);
        }

        void Draw() const { mProgram.Draw(); }
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

        void SetModelMatrix(const glm::mat4& model) const override {}
    };

    ////////////////////////////////////////
    struct EmissiveColorProgram : public AbstractEmissiveColorProgram
    {
        EmissiveColorProgram(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& model) const override
        { glUniformMatrix4fv(MODEL_LOC, 1, GL_FALSE, glm::value_ptr(model)); }
    };

    ////////////////////////////////////////
    struct EmissiveTextureMaterial
    {
        glm::vec2 mTileMultiplier;
        glm::vec2 mTileOffset;
    };

    ////////////////////////////////////////
    struct AbstractEmissiveTextureProgram
    {
    protected:
        Program mProgram;

    public:
        AbstractEmissiveTextureProgram(const std::string& rootPath, ShaderLoader&, const std::string& vshaderUrl);

        virtual ~AbstractEmissiveTextureProgram() {}

        static inline constexpr int EMISSIVE_TEXTURE_LOC = 0;
        static inline constexpr int TILE_MULTIPLIER_LOC = 1;
        static inline constexpr int TILE_OFFSET_LOC = 2;
        static inline constexpr int MODEL_LOC = 3;

        void SetMaterial(const EmissiveTextureMaterial& m) const
        {
            glUniform2fv(TILE_MULTIPLIER_LOC, 1, glm::value_ptr(m.mTileMultiplier));
            glUniform2fv(TILE_OFFSET_LOC, 1, glm::value_ptr(m.mTileOffset));
        }

        virtual void SetModelMatrix(const glm::mat4& model) const = 0;

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }
    };

    ////////////////////////////////////////
    struct EmissiveTextureProgramInstanced : public AbstractEmissiveTextureProgram
    {
        EmissiveTextureProgramInstanced(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& model) const override {}
    };

    ////////////////////////////////////////
    struct EmissiveTextureProgram : public AbstractEmissiveTextureProgram
    {
        EmissiveTextureProgram(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& model) const override
        { glUniformMatrix4fv(MODEL_LOC, 1, GL_FALSE, glm::value_ptr(model)); }
    };

    ////////////////////////////////////////
    enum class DefaultSkyboxTexture{ Cloudy,
                                     Apocalypse,
                                     ApocalypseLand,
                                     ApocalypseOcean,
                                     Classic,
                                     ClassicLand,
                                     Clear,
                                     ClearOcean,
                                     Dawn,
                                     Dusk,
                                     DuskLand,
                                     DuskOcean,
                                     EmptySpace,
                                     Gray,
                                     Moody,
                                     Netherworld,
                                     Sinister,
                                     SinisterLand,
                                     SinisterOcean,
                                     Sunshine,
                                     Techno };

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
    struct AbstractBlinnPhongProgram
    {
    protected:
        Program mProgram;

    public:
        AbstractBlinnPhongProgram(const std::string& rootPath, ShaderLoader&, const std::string&);

        virtual ~AbstractBlinnPhongProgram() {}

        static inline constexpr int MODEL_MATRIX_LOC{ 0 };
        static inline constexpr int NORMAL_MATRIX_LOC{ 1 };
        static inline constexpr int TEX_OFFSET_LOC{ 2 };
        static inline constexpr int TEX_MULTIPLIER_LOC{ 3 };
        static inline constexpr int MATERIAL_AMBIENT_TEXTURE_LOC{ 4 };
        static inline constexpr int MATERIAL_DIFFUSE_TEXTURE_LOC{ 5 };
        static inline constexpr int MATERIAL_SPECULAR_TEXTURE_LOC{ 6 };
        static inline constexpr int AMBIENT_FACTOR_LOC{ 7 };

        void Use() const { mProgram.Use(); }
        void Halt() const { mProgram.Halt(); }

        virtual void SetModelMatrix(const glm::mat4& modelMatrix) const = 0;
        virtual void SetNormalMatrix(const glm::mat3& normalMatrix) const = 0;

        void SetTexOffset(const glm::vec2& texOffset) const
        { glUniform2fv(TEX_OFFSET_LOC, 1, glm::value_ptr(texOffset)); }

        void SetTexMultiplier(const glm::vec2& texMultiplier) const
        { glUniform2fv(TEX_MULTIPLIER_LOC, 1, glm::value_ptr(texMultiplier)); }

        void SetAmbientFactor(float factor) const
        { glUniform1f(AMBIENT_FACTOR_LOC, factor); }
    };

    ////////////////////////////////////////
    struct BlinnPhongProgram : public AbstractBlinnPhongProgram
    {
        BlinnPhongProgram(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& modelMatrix) const override
        { glUniformMatrix4fv(MODEL_MATRIX_LOC, 1, GL_FALSE, glm::value_ptr(modelMatrix)); }

        void SetNormalMatrix(const glm::mat3& normalMatrix) const override
        { glUniformMatrix3fv(NORMAL_MATRIX_LOC, 1, GL_FALSE, glm::value_ptr(normalMatrix)); }
    };

    ////////////////////////////////////////
    struct BlinnPhongProgramInstanced : public AbstractBlinnPhongProgram
    {
        BlinnPhongProgramInstanced(const std::string& rootPath, ShaderLoader&);

        void SetModelMatrix(const glm::mat4& modelMatrix) const override {}
        void SetNormalMatrix(const glm::mat3& normalMatrix) const override {}
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
