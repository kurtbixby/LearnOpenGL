#include "Headers/Mesh.h"

#include <vector>

#include "Headers/Structs.h"
#include "Headers/Shader.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices)
{
	vertices_ = vertices;
	indices_ = indices;

	initializeMesh();
}

void Mesh::Draw(Shader shader, vector<Texture> textures)
{
    PrepareTextures(shader, textures);

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawInstanced(Shader shader, vector<Texture> textures, vector<glm::mat4> instance_matrices)
{
    glBindVertexArray(vao_);
    PrepareTextures(shader, textures);
    PrepareMatrices(instance_matrices);
    
    glDrawElementsInstanced(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0, instance_matrices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

void Mesh::PrepareTextures(Shader& shader, vector<Texture>& textures)
{
    unsigned int diffuseTexs = 0;
    unsigned int specularTexs = 0;
    unsigned int reflectionMaps = 0;
    
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
        }
    }
    shader.SetInt("DIFFUSE_TEXS", diffuseTexs);
    shader.SetInt("SPECULAR_TEXS", specularTexs);
    shader.SetInt("REFLECTION_MAPS", reflectionMaps);
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
