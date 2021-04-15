#include "components.h"
#include "app.h"

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
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.texture->getID());
                glUniform1i(loc, 0);
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
        cam.m_viewProj = cam.m_projection * trans.getMatrix();
    }
}