#pragma once

#include <memory>
#include "glm/glm.hpp"
#include "entt/entt.hpp"
#include "mesh.h"
#include "shader.h"
#include "transform.h"
#include "texture.h"

struct MeshRenderComponent {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;
    std::weak_ptr<Texture> texture;
};

void RenderMeshComponents(entt::registry& r);

class CameraComponent {
public:
    CameraComponent(const float fov, const float near, const float far);
    const glm::mat4& GetViewProjMatrix() const;
    static void Update(entt::registry& r);
protected:
    glm::mat4 m_projection;
    glm::mat4 m_viewProj;
};

struct MouseLook {
    float yaw;
    float pitch;
    float pitch_limit;
    float sensitivity;
    float move_speed;
};

void UpdateMouseLook(entt::registry& r, const float deltaTime);