#include "transform.h"

Transform::Transform(const glm::vec3 pos, const glm::quat rot, const glm::vec3 scale)
     : m_pos(pos), m_rot(rot), m_scale(scale) {
    m_dirty = true;
    m_matrix = getMatrix();
}

Transform::Transform(const glm::vec3 pos, const glm::quat rot, const float scale)
    : Transform(pos, rot, glm::vec3(scale)) {
}

Transform::Transform(const glm::vec3 pos)
    : Transform(pos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f) {
}

Transform::Transform() 
    : Transform(glm::vec3(0.0f)) {
}

glm::mat4x4& Transform::getMatrix() {
    if (m_dirty) {
        m_dirty = false;
        m_matrix = glm::translate(m_pos) * glm::mat4x4(m_rot) * glm::scale(m_scale);
    }
    return m_matrix;
}

glm::vec3 Transform::getPos() {
    return m_pos;
}

glm::vec3& Transform::setPos(const glm::vec3 pos) {
    m_pos = pos;
    m_dirty = true;
    return m_pos;
}

glm::vec3& Transform::setPos() {
    m_dirty = true;
    return m_pos;
}

glm::quat Transform::getRot() {
    return m_rot;
}

glm::quat& Transform::setRot(const glm::quat rot) {
    m_rot = rot;
    m_dirty = true;
    return m_rot;
}

glm::quat& Transform::setRot() {
    m_dirty = true;
    return m_rot;
}

glm::vec3 Transform::getScale() {
    return m_scale;
}

glm::vec3& Transform::setScale(const glm::vec3 scale) {
    m_scale = scale;
    m_dirty = true;
    return m_scale;
}

glm::vec3& Transform::setScale(const float scale) {
    return Transform::setScale(glm::vec3(scale));
}

glm::vec3& Transform::setScale() {
    m_dirty = true;
    return m_scale;
}

bool Transform::isDirty() const {
    return m_dirty;
}

void Transform::rotateX(const float delta) {
    rotateAxisAngle(glm::vec3(1.0f, 0.0f, 0.0f), delta);
}

void Transform::rotateY(const float delta) {
    rotateAxisAngle(glm::vec3(0.0f, 1.0f, 0.0f), delta);
}

void Transform::rotateZ(const float delta) {
    rotateAxisAngle(glm::vec3(0.0f, 0.0f, 1.0f), delta);
}

void Transform::translate(const glm::vec3& delta) {
    m_pos += delta;
    m_dirty = true;
}

void Transform::translate(const float x, const float y, const float z) {
    m_pos.x += x;
    m_pos.y += y;
    m_pos.z += z;
    m_dirty = true;
}

void Transform::rotateAxisAngle(const glm::vec3 axis, const float delta) {
    m_rot *= glm::angleAxis(delta, axis);
    m_dirty = true;
}