#pragma once

#include "node.h"

#ifndef CAMERA_H
#define CAMERA_H

class Camera : Node
{
public:
    //Fov is in degrees
    Camera(const float fov, const float aspectRatio, const float near, const float far);

    inline const glm::mat4x4 getViewProjectionMatrix()
    {
        if (transformDirty) updateTransform();
        return glm::mat4x4(viewProjMatrix);
    }

protected:
    void updateTransform() override;

private:
    glm::mat4x4 projMatrix;
    glm::mat4x4 viewProjMatrix;
};

#endif