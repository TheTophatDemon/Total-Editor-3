#include "mesh.h"

Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) 
    : vertexData(std::move(vertices)), indexData(std::move(indices))
{
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glCreateBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    
    glCreateBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(short), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(IDX_POSITION, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_POSITION);
    glVertexAttribPointer(IDX_UV, 2, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_UV);
    glVertexAttribPointer(IDX_COLOR, 4, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_COLOR);
    glVertexAttribPointer(IDX_NORMAL, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_NORMAL);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    triCount = indexData.size() / 3;
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

void Mesh::bind()
{
    glBindVertexArray(vertexArrayObject);
}