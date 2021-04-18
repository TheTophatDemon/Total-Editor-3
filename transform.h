#pragma once

#include "glm/mat4x4.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

class Transform {
public:
    Transform();
    Transform(const glm::vec3 pos);
    Transform(const glm::vec3 pos, const glm::quat rot, const float scale = 1.0f);
    Transform(const glm::vec3 pos, const glm::quat rot, const glm::vec3 scale);
    glm::mat4x4& getMatrix();
    glm::vec3 getPos();
    glm::vec3& setPos(const glm::vec3 pos);
    glm::vec3& setPos();
    glm::quat getRot();
    glm::quat& setRot(const glm::quat rot);
    glm::quat& setRot();
    glm::vec3 getScale();
    glm::vec3& setScale(const glm::vec3 scale);
    glm::vec3& setScale(const float scale);
    glm::vec3& setScale();
    void rotateX(const float delta);
    void rotateY(const float delta);
    void rotateZ(const float delta);
    void translate(const glm::vec3& delta);
    void translate(const float x, const float y, const float z);
    bool isDirty() const;
protected:
    void rotateAxisAngle(const glm::vec3 axis, const float delta);
    glm::vec3 m_pos;
    glm::quat m_rot;
    glm::vec3 m_scale;
    glm::mat4x4 m_matrix;
    bool m_dirty;
};