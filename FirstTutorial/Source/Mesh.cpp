#include "Headers/Mesh.h"

#include <vector>

#include "Headers/Structs.h"
#include "Headers/Shader.h"
#include "Headers/Texture.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices)
{
	vertices_ = vertices;
	indices_ = indices;

	initializeMesh();
}

void Mesh::Draw(const Shader& shader, vector<Texture> textures)
{
    PrepareTextures(shader, textures);
    PrepareTangentSpace();
    
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawInstanced(const Shader& shader, vector<Texture> textures, vector<glm::mat4> instance_matrices)
{
    glBindVertexArray(vao_);
    PrepareTextures(shader, textures);
    PrepareMatrices(instance_matrices);
    
    glDrawElementsInstanced(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0, instance_matrices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

void Mesh::PrepareTextures(const Shader& shader, vector<Texture>& textures)
{
    unsigned int diffuseTexs = 0;
    unsigned int specularTexs = 0;
    unsigned int reflectionMaps = 0;
    unsigned int normalMaps = 0;
    
    for (int i = 0; i < textures.size(); i++)
    {
        std::string texName = "material.";
        Texture tex = textures[i];
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        switch(tex.type)
        {
            case TextureType::Diffuse:
            texName.append("diffuse[").append(std::to_string(diffuseTexs)).append("]");
                shader.SetInt(texName, i);
                diffuseTexs += 1;
                break;
            case TextureType::Specular:
            texName.append("specular[").append(std::to_string(specularTexs)).append("]");
                shader.SetInt(texName, i);
                specularTexs += 1;
                break;
            case TextureType::Reflection:
            texName.append("reflection[").append(std::to_string(reflectionMaps)).append("]");
                shader.SetInt(texName, i);
                reflectionMaps += 1;
                break;
                
            case TextureType::Normal:
                texName.append("normals[").append(std::to_string(reflectionMaps)).append("]");
                shader.SetInt(texName, i);
                normalMaps += 1;
                break;
        }
    }
    shader.SetInt("DIFFUSE_TEXS", diffuseTexs);
    shader.SetInt("SPECULAR_TEXS", specularTexs);
    shader.SetInt("REFLECTION_MAPS", reflectionMaps);
}

void Mesh::PrepareTangentSpace()
{
    std::vector<glm::vec3> tanSpaceVectors = std::vector<glm::vec3>();
    for (int i = 0; i < vertices_.size() - 2; i += 3)
    {
        Vertex v0 = vertices_[i];
        Vertex v1 = vertices_[i + 1];
        Vertex v2 = vertices_[i + 2];
        glm::vec3 e1 = v1.Position - v0.Position;
        glm::vec3 e2 = v2.Position - v0.Position;
        glm::vec2 dUV1 = v1.TexCoords - v0.TexCoords;
        glm::vec2 dUV2 = v2.TexCoords - v0.TexCoords;
        
        float coef = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
        glm::vec3 tan = glm::vec3(0.0f);
        tan.x = dUV2.y * e1.x - dUV1.y * e2.x;
        tan.y = dUV2.y * e1.y - dUV1.y * e2.y;
        tan.z = dUV2.y * e1.z - dUV1.y * e2.z;
        tan = normalize(coef * tan);
        
        glm::vec3 bitan = glm::vec3(0.0f);
        bitan.x = -dUV2.x * e1.x + dUV1.x * e2.x;
        bitan.y = -dUV2.x * e1.y + dUV1.x * e2.y;
        bitan.z = -dUV2.x * e1.z + dUV1.x * e2.z;
        bitan = normalize(coef * bitan);
        
        // Push each back 3 times for the 3 vertices
        tanSpaceVectors.push_back(tan);
        tanSpaceVectors.push_back(bitan);
        tanSpaceVectors.push_back(tan);
        tanSpaceVectors.push_back(bitan);
        tanSpaceVectors.push_back(tan);
        tanSpaceVectors.push_back(bitan);
    }
    
    uint32_t tan_space;
    glGenBuffers(1, &tan_space);
    glBindBuffer(GL_ARRAY_BUFFER, tan_space);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * 2 * sizeof(glm::vec3), &tanSpaceVectors[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (GLvoid*)sizeof(glm::vec3));
}

void Mesh::PrepareMatrices(vector<glm::mat4>& instance_matrices)
{
    uint32_t matrix_buffer;
    glGenBuffers(1, &matrix_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
    glBufferData(GL_ARRAY_BUFFER, instance_matrices.size() * sizeof(glm::mat4), &instance_matrices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), nullptr);
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)sizeof(glm::vec4));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(5, 1);
    
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(6, 1);
}

void Mesh::initializeMesh()
{
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ebo_);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));

    glBindVertexArray(0);
}
