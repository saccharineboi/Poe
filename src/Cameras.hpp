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
ENABLE_WARNINGS()

#include <vector>

namespace Poe
{
    ////////////////////////////////////////
    enum class CameraProjectionType { Orthographic, Perspective };

    ////////////////////////////////////////
    struct AbstractCamera
    {
    protected:
        CameraProjectionType mProjectionType;

        glm::mat4 mProjectionMatrix;
        glm::mat4 mViewMatrix;

    public:

        explicit AbstractCamera(CameraProjectionType type);

        virtual ~AbstractCamera() {}

        glm::mat4 GetProjectionMatrix() const { return mProjectionMatrix; }
        glm::mat4 GetViewMatrix() const { return mViewMatrix; }
        std::vector<glm::vec4> GetFrustumCornersInWorldSpace(float near, float far) const;

        virtual float GetFovy() const = 0;
        virtual float GetAspectRatio() const = 0;
        virtual float GetNear() const = 0;
        virtual float GetFar() const = 0;
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
    struct FirstPersonCamera : public AbstractCamera
    {
    private:
        FirstPersonCameraState mState;
        FirstPersonCameraInputConfig mInputConfig;

    public:
        bool mIsMouseCaptured;

        glm::vec3 mPosition;
        glm::vec3 mDirection;
        glm::vec3 mUp;

        glm::vec3 mTargetPosition;

        float mFovy;
        float mAspectRatio;
        float mNear;
        float mFar;

        float mSpeed;
        float mSensitivity;
        float mSmoothness;

        FirstPersonCamera();

        void UpdateInputConfig(int key, int action);
        void UpdateDirection(float mouseX, float mouseY);
        void Update(float dt);

        void SetAspectRatio(int width, int height)
        { mAspectRatio = static_cast<float>(width) / static_cast<float>(height); }

        float GetFovy() const override { return mFovy; }
        float GetAspectRatio() const override { return mAspectRatio; }
        float GetNear() const override { return mNear; }
        float GetFar() const override { return mFar; }
    };
}
