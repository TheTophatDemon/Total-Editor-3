#include "components.h"

void RenderMeshComponents(entt::registry& r) {
    auto view = r.view<MeshRenderComponent, Transform>();
    for (auto [ent, mesh, trans]: view.each()) {
    
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