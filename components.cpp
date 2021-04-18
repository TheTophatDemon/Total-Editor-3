#include "components.h"
#include "app.h"
#include "input.h"
#include <algorithm>

void RenderMeshComponents(entt::registry& r) {
    auto mesh_view = r.view<MeshRenderComponent, Transform>();
    for (auto [cam_ent, cam]: r.view<CameraComponent>().each()) {
        //Renders for each camera in the registry (should be only one for now...)
        for (auto [ent, mesh, trans]: mesh_view.each()) {
            mesh.shader->bind();

            if (int loc = mesh.shader->getUniformLoc("uViewProjMat"); loc >= 0) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, &cam.GetViewProjMatrix()[0][0]);
            }

            if (int loc = mesh.shader->getUniformLoc("uModelMat"); loc >= 0) {
                glm::mat4x4 modelMat = trans.getMatrix();
                glUniformMatrix4fv(loc, 1, GL_FALSE, &modelMat[0][0]);
            }

            if (int loc = mesh.shader->getUniformLoc("uTexture"); loc >= 0) {
                if (std::shared_ptr<Texture> tex = mesh.texture.lock()) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, tex->getID());
                    glUniform1i(loc, 0);
                }
            }

            mesh.mesh->render();
        }
    }
}

CameraComponent::CameraComponent(const float fov, const float near, const float far) {
    m_projection = glm::perspectiveFov<float>(70.0f, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, 0.1f, 100.0f);
    m_viewProj = m_projection;
}

const glm::mat4& CameraComponent::GetViewProjMatrix() const {
    return m_viewProj;
}

//Updates the view matrices of the cameras based off of position
void CameraComponent::Update(entt::registry& r) {
    auto view = r.view<Transform, CameraComponent>();
    for (auto [ent, trans, cam] : view.each()) {
        cam.m_viewProj = cam.m_projection * glm::inverse(trans.getMatrix());
    }
}

void UpdateMouseLook(entt::registry& r, const float deltaTime) {
    auto view = r.view<Transform, MouseLook>();
    for (auto [ent, trans, look] : view.each()) {
        if (Input::isMouseButtonDown(Input::MouseButton::RIGHT)) {
            glm::vec2 mous = Input::getMouseMovement();
            look.yaw += mous.x * look.sensitivity;
            look.pitch = std::max(-look.pitch_limit, std::min(look.pitch_limit, look.pitch + mous.y * look.sensitivity));
            auto newRot = glm::angleAxis(-look.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            newRot *= glm::angleAxis(-look.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            trans.setRot(newRot);
        }
        
        glm::vec3 movement = glm::vec3(0.0f, 0.0f, 0.0f);
        if (Input::isKeyDown(SDL_SCANCODE_W)) {
            movement.z = -1.0f;
        } else if (Input::isKeyDown(SDL_SCANCODE_S)) {
            movement.z = 1.0f;
        }
        if (Input::isKeyDown(SDL_SCANCODE_D)) {
            movement.x = 1.0f;
        } else if (Input::isKeyDown(SDL_SCANCODE_A)) {
            movement.x = -1.0f;
        }
        movement = trans.getRot() * movement;
        if (Input::isKeyDown(SDL_SCANCODE_C)) {
            movement.y = -1.0f;
        } else if (Input::isKeyDown(SDL_SCANCODE_SPACE)) {
            movement.y = 1.0f;
        }
        if (movement.x + movement.y + movement.z != 0.0f) {
            movement = glm::normalize(movement);
        }
        trans.translate(movement * look.move_speed * deltaTime);
    }
}