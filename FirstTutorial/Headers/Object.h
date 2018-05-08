#ifndef OBJECT_H
#define OBJECT_H

#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Model.h"
#include "Shader.h"

class Object
{
public:
    std::string Model_;
    bool Outlined_;
    bool Is2D_;
    bool IsTransparent_;
    
    // Have a tree-like structure for

    Object() : Object(glm::vec3(1.0f), "Defaults/box", glm::vec3(1.0f), false) {};
    Object(const glm::vec3 transform, std::string model, const glm::vec3 scale, const bool outline = false, const bool is2D = false, const bool isTransparent = false)
    {
        Transform_ = transform;
        Model_ = model;
        Scale_ = scale;
        Outlined_ = outline;
        Is2D_ = is2D;
        IsTransparent_ = isTransparent;
    }
    
    glm::mat4 ModelMatrix() const
    {
        return glm::scale(glm::translate(glm::mat4(1.0f), Transform_), Scale_);
    }
    
    glm::vec3 Transform() const
    {
        return Transform_;
    }

    void Draw(Shader shader, Model& model) const
    {
        if (Is2D_)
        {
            glDisable(GL_CULL_FACE);
        }
        model.Draw(shader);
        if (Is2D_)
        {
            glEnable(GL_CULL_FACE);
        }
    }
    
private:
    // Have tree of model matrices for animation
    glm::vec3 Transform_;
    glm::vec3 Scale_;
    glm::vec3 Rotation_; // Change this to quaternion
    
};

#endif
