#pragma once

#include <glew.h>
#include <GL/gl.h>
#include <vector>
#include <memory>

class Mesh
{
public:
    const constexpr static int IDX_POSITION = 0;
    const constexpr static int IDX_UV = 1;
    const constexpr static int IDX_COLOR = 2;
    const constexpr static int IDX_NORMAL = 3;
    const constexpr static int OFS_POSITION = 0;
    const constexpr static int OFS_UV = sizeof(float) * 3;
    const constexpr static int OFS_COLOR = OFS_UV + sizeof(float) * 2;
    const constexpr static int OFS_NORMAL = OFS_COLOR + sizeof(float) * 4;
    const constexpr static int ATTRIB_STRIDE = OFS_NORMAL + sizeof(float) * 3;

    Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices);
    ~Mesh();
    void bind();
    inline int getTriCount() { return triCount; }
    inline int getIndexCount() { return indexData.size(); }
protected:
    std::vector<float> vertexData;
    std::vector<unsigned short> indexData;
    
    GLuint vertexBuffer;
    GLuint vertexArrayObject;
    GLuint indexBuffer;
    int triCount;
};