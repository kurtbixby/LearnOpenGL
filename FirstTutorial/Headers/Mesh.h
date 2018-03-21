#ifndef MESH_H
#define MESH_H

#include <vector>

#include "Structs.h"
#include "Shader.h"

using namespace std;

class Mesh
{
public:
    vector<Vertex> vertices_;
    vector<unsigned int> indices_;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices);
    void Draw(Shader shader, vector<Texture> textures);
    void DrawInstanced(Shader shader, vector<Texture> textures, vector<glm::mat4> instance_matrices);
private:
    unsigned int vao_, vbo_, ebo_;
    void initializeMesh();
    
    void PrepareTextures(Shader& shader, vector<Texture>& textures);
    void PrepareTangentSpace();
    void PrepareMatrices(vector<glm::mat4>& instance_matrices);
};

#endif
