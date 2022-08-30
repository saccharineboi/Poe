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

#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <string>

namespace Poe
{
    ////////////////////////////////////////
    inline constexpr float PI = 3.1415926f;
    inline constexpr float PI2 = 6.2831853f;
    inline constexpr float R2D = 57.295779f;
    inline constexpr float D2R = 0.017453293f;

    ////////////////////////////////////////
    void APIENTRY GraphicsDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    ////////////////////////////////////////
    int Run();

    ////////////////////////////////////////
    struct ObjectGL
    {
    protected:
        unsigned mId;

    public:
        ObjectGL() {}
        ~ObjectGL() {}

        unsigned GetId() const { return mId; }

        ObjectGL(const ObjectGL&) = delete; ObjectGL& operator=(const ObjectGL&) = delete;
    };

    ////////////////////////////////////////
    struct BufferGL : public ObjectGL
    {
    protected:
        int mMode;
        std::size_t mNumElements;

    public:
        ~BufferGL() {}

        int GetMode() const { return mMode; }
        std::size_t GetNumElements() const { return mNumElements; }
    };

    ////////////////////////////////////////
    struct VertexBuffer : public BufferGL
    {
        VertexBuffer(const std::vector<float>& vertices, int mode);

        ~VertexBuffer() { glDeleteBuffers(1, &mId); }

        VertexBuffer(VertexBuffer&&);
        VertexBuffer& operator=(VertexBuffer&&);

        void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, mId); }
        void UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
    };

    ////////////////////////////////////////
    struct IndexBuffer : public BufferGL
    {
        IndexBuffer(const std::vector<unsigned>& indices, int mode);

        ~IndexBuffer() { glDeleteBuffers(1, &mId); }

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

    public:
        Program(const std::initializer_list<const Shader*>& shaders);

        ~Program() { glDeleteProgram(mId); }

        Program(Program&&);
        Program& operator=(Program&&);

        void Use() const { glUseProgram(mId); }
        void Halt() const { glUseProgram(0); }
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

    ////////////////////////////////////////
    StaticMesh CreateColoredQuad();

    ////////////////////////////////////////
    StaticMesh CreateColoredCircle(float radius, int numSegments);
}
