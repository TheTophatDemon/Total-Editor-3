#pragma once

#include "glew/include/glew.h"
#include <GL/gl.h>
#include <vector>
#include <memory>

class Mesh
{
public:
    Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices);
    virtual ~Mesh();
    virtual void render() = 0;
    int getIndexCount() const;
protected:
    std::vector<float> m_vertexData;
    std::vector<unsigned short> m_indexData;
    GLuint m_vertexBuffer;
    GLuint m_vertexArrayObject;
    GLuint m_indexBuffer;
    unsigned m_indexCount;
};

class TriMesh : public Mesh {
public:
    constexpr static int IDX_POSITION = 0;
    constexpr static int IDX_UV = 1;
    constexpr static int IDX_COLOR = 2;
    constexpr static int IDX_NORMAL = 3;
    constexpr static int OFS_POSITION = 0;
    constexpr static int OFS_UV = sizeof(float) * 3;
    constexpr static int OFS_COLOR = OFS_UV + sizeof(float) * 2;
    constexpr static int OFS_NORMAL = OFS_COLOR + sizeof(float) * 4;
    constexpr static int ATTRIB_STRIDE = OFS_NORMAL + sizeof(float) * 3;

    TriMesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices);
    virtual void render() override;
};

class LineMesh : public Mesh {
public:
    constexpr static int IDX_POSITION = 0;
    constexpr static int IDX_COLOR = 1;
    constexpr static int OFS_POSITION = 0;
    constexpr static int OFS_COLOR = sizeof(float) * 3;
    constexpr static int ATTRIB_STRIDE = OFS_COLOR + sizeof(float) * 4;

    LineMesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices);
    virtual void render() override;
};