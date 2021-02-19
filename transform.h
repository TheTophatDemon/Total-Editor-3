#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

class Transform {
public:
    Transform();
    Transform(const glm::vec3 pos);
    Transform(const glm::vec3 pos, const glm::quat rot, const float scale = 1.0f);
    Transform(const glm::vec3 pos, const glm::quat rot, const glm::vec3 scale);
    glm::mat4x4& GetMatrix();
    glm::vec3 GetPos();
    glm::vec3& SetPos(const glm::vec3 pos);
    glm::vec3& SetPos();
    glm::quat GetRot();
    glm::quat& SetRot(const glm::quat rot);
    glm::quat& SetRot();
    glm::vec3 GetScale();
    glm::vec3& SetScale(const glm::vec3 scale);
    glm::vec3& SetScale(const float scale);
    glm::vec3& SetScale();
protected:
    glm::vec3 m_pos;
    glm::quat m_rot;
    glm::vec3 m_scale;
    glm::mat4x4 m_matrix;
    bool m_dirty;
};