#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#ifndef NODE_H
#define NODE_H

//Represents a thing in space with position, rotation, and scale
class Node
{
public:
    Node();
    Node(glm::vec3 pos, glm::quat rot, glm::vec3 sca);
    Node(glm::vec3 pos);
    Node(glm::vec3 pos, glm::quat rot);

    virtual void update(const float deltaTime) = 0;

    //Returns a copy of the node's world transform
    const inline glm::mat4x4 getTransform()
    {
        if (transformDirty) updateTransform();
        return glm::mat4x4(transform);
    }
    
    //Returns a copy of the node's position
    const inline glm::vec3 getPosition() const
    {
        return glm::vec3(position);
    }

    //Returns a copy of the node's rotation
    const inline glm::quat getRotation() const
    {
        return glm::quat(rotation);
    }

    //Returns a copy of the node's scale
    const inline glm::vec3 getScale() const
    {
        return glm::vec3(scale);
    }

    const inline glm::vec3 setPosition(const glm::vec3&& newPos)
    {
        position = glm::vec3(newPos);
        transformDirty = true;
    }

    const inline glm::vec3 setScale(const glm::vec3&& newScale)
    {
        scale = glm::vec3(newScale);
        transformDirty = true;
    }

    const inline glm::quat setRotation(const glm::quat&& newRot)
    {
        rotation = glm::quat(newRot);
        transformDirty = true;
    }

protected:
    virtual void updateTransform();

    glm::mat4x4 transform;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    //Set to true when the transform needs to be updated.
    bool transformDirty;
};

#endif