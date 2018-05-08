#ifndef MESH_H
#define MESH_H

#include <vector>

#include "ModelTexture.h"
#include "Structs.h"
#include "Shader.h"

using namespace std;

class Mesh
{
public:
    vector<Vertex> vertices_;
    vector<unsigned int> indices_;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices);
    void Draw(const Shader& shader, vector<ModelTexture> textures);
    void Draw_NormalMapped(const Shader& shader, vector<ModelTexture> textures);
    void DrawInstanced(const Shader& shader, vector<ModelTexture> textures, vector<glm::mat4> instance_matrices);
private:
    unsigned int vao_, vbo_, ebo_;
    void initializeMesh();
    
    void PrepareTextures(const Shader& shader, vector<ModelTexture>& textures);
    void PrepareTangentSpace();
    void PrepareMatrices(vector<glm::mat4>& instance_matrices);
};

#endif
