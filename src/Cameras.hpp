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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_INLINE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace Poe
{
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

        float mSpeed = 100.0f;
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

        void SetAspectRatio(int width, int height);
        void SetPosition(const glm::vec3& position);
    };
}
