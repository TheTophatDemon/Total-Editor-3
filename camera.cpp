#include "camera.h"

Camera::Camera(const float fov, const float aspectRatio, const float near, const float far)
    : Node()
{
    projMatrix = glm::perspective(glm::pi<float>() * fov / 180.0f, aspectRatio, near, far);
}

void Camera::updateTransform()
{
    Node::updateTransform();

    viewProjMatrix = projMatrix * transform;
}