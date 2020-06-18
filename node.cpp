#include "node.h"

Node::Node(glm::vec3 pos, glm::quat rot, glm::vec3 sca)
    : position(pos), rotation(rot), scale(sca)
{
    transformDirty = true;
}

Node::Node(glm::vec3 pos, glm::quat rot)
    : Node(pos, rot, glm::vec3(1.0f)) 
{}

Node::Node(glm::vec3 pos)
    : Node(pos, glm::quat(), glm::vec3(1.0f)) 
{}

Node::Node()
    : Node(glm::vec3(0.0f))
{}

void Node::updateTransform()
{
    transform = glm::translate(position)
     * glm::mat4_cast(rotation)
     * glm::scale(scale);

    transformDirty = false;
}