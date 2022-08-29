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

#include "Poe.hpp"

#include <cstdio>

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
}
