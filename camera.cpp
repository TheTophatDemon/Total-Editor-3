#include "camera.h"

Camera::Camera(const glm::vec3 pos, const glm::quat rot, const float fov, const float aspectRatio, const float near, const float far)
    : Node(pos, rot), 
    mode(CameraMode::ORBIT)
{
    projMatrix = glm::perspective(glm::pi<float>() * fov / 180.0f, aspectRatio, near, far);
}

Camera::Camera(const float fov, const float aspectRatio, const float near, const float far)
    : Camera(glm::vec3(0.0f), glm::quat(), fov, aspectRatio, near, far)
{
}

void Camera::update(const float deltaTime)
{
    
}

void Camera::updateTransform()
{
    Node::updateTransform();

    viewProjMatrix = projMatrix * transform;
}