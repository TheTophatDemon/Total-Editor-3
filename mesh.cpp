#include "mesh.h"

Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) 
    : m_vertexData(std::move(vertices)), m_indexData(std::move(indices))
{
    glGenVertexArrays(1, &m_vertexArrayObject);
    glBindVertexArray(m_vertexArrayObject);
    
    glCreateBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float), m_vertexData.data(), GL_STATIC_DRAW);
    
    glCreateBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexData.size() * sizeof(short), m_indexData.data(), GL_STATIC_DRAW);

    m_indexCount = m_indexData.size();
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

int Mesh::getIndexCount() const {
    return m_indexCount;
}

TriMesh::TriMesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) : Mesh(std::move(vertices), std::move(indices)) {
}

void TriMesh::render() {
    glBindVertexArray(m_vertexArrayObject);

    glVertexAttribPointer(IDX_POSITION, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_POSITION);
    glVertexAttribPointer(IDX_UV, 2, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_UV);
    glVertexAttribPointer(IDX_COLOR, 4, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_COLOR);
    glVertexAttribPointer(IDX_NORMAL, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_NORMAL);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_SHORT, 0);
}

LineMesh::LineMesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) : Mesh(std::move(vertices), std::move(indices)) {
}


void LineMesh::render() {
    glBindVertexArray(m_vertexArrayObject);

    glVertexAttribPointer(IDX_POSITION, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_POSITION);
    glVertexAttribPointer(IDX_COLOR, 4, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_COLOR);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINE, getIndexCount(), GL_UNSIGNED_SHORT, 0);
}