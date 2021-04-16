#include "mesh.h"

Mesh::Mesh() {
}

Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) 
    : m_vertexData(std::move(vertices)), m_indexData(std::move(indices))
{
    generateOpenGLArrays();
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

void Mesh::generateOpenGLArrays() {
    glGenVertexArrays(1, &m_vertexArrayObject);
    glBindVertexArray(m_vertexArrayObject);
    
    glCreateBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float), m_vertexData.data(), GL_STATIC_DRAW);
    
    glCreateBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexData.size() * sizeof(short), m_indexData.data(), GL_STATIC_DRAW);
}

int Mesh::getIndexCount() const {
    return m_indexData.size();
}

TriMesh::TriMesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices) : Mesh(std::move(vertices), std::move(indices)) {
}

void TriMesh::render() {
    glBindVertexArray(m_vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

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

LineMesh::LineMesh(std::vector<glm::vec3>& vertices, std::vector<unsigned short>& indices) {
    //Decompose vertices into float sequence
    m_vertexData = std::vector<float>();
    for (auto i = vertices.begin(); i != vertices.end(); i++) {
        m_vertexData.push_back(i->x);
        m_vertexData.push_back(i->y);
        m_vertexData.push_back(i->z);
        for (int j = 0; j < 4; j++) m_vertexData.push_back(1.0f); //Color
    }
    m_indexData = std::vector<unsigned short>(indices);
    generateOpenGLArrays();
}


void LineMesh::render() {
    glBindVertexArray(m_vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glVertexAttribPointer(IDX_POSITION, 3, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_POSITION);
    glVertexAttribPointer(IDX_COLOR, 4, GL_FLOAT, false, ATTRIB_STRIDE, (const void*) OFS_COLOR);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    
    glDrawElements(GL_LINES, getIndexCount(), GL_UNSIGNED_SHORT, 0);
}