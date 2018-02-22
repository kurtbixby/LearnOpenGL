#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "Mesh.h"
#include "Shader.h"

class Model
{
public:
    Model(std::vector<Mesh> meshes, std::vector<std::vector<Texture>> mesh_textures);
    void Draw(const Shader& shader);
    void DrawInstanced(const Shader& shader, std::vector<glm::mat4> instance_matrices);

private:
    // Have a tree of meshes for animation
    std::vector<Mesh> meshes_;
    std::vector<std::vector<Texture>> mesh_textures_;
};

#endif
