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

#include "Cameras.hpp"
#include "Utility.hpp"
#include "Constants.hpp"

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

namespace Poe
{
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
    void FirstPersonCamera::SetAspectRatio(int width, int height)
    {
        mAspectRatio = static_cast<float>(width) / static_cast<float>(height);
    }

    ////////////////////////////////////////
    void FirstPersonCamera::SetPosition(const glm::vec3& position)
    {
        mPosition = mTargetPosition = position;
    }

}
