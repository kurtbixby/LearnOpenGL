#include "Mesh.h"

#include <vector>

#include "Structs.h"
#include "Shader.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
{
	vertices_ = vertices;
	indices_ = indices;
	textures_ = textures;

	initializeMesh();
}

void Mesh::Draw(Shader shader)
{
	unsigned int diffuseTexs = 0;
	unsigned int specularTexs = 0;

	for (int i = 0; i < textures_.size(); i++)
	{
		std::string texName = "material.";
		Texture tex = textures_[i];
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
		}
	}
	shader.SetInt("DIFFUSE_TEXS", diffuseTexs);
	shader.SetInt("SPECULAR_TEXS", specularTexs);

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
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
