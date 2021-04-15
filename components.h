#pragma once

#include <memory>
#include "entt/entt.hpp"
#include "mesh.h"
#include "shader.h"
#include "transform.h"
#include "texture.h"

struct MeshRenderComponent {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
};

void RenderMeshComponents(entt::registry& r);