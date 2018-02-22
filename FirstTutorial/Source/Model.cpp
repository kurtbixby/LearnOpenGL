#include "Headers/Model.h"

#include <vector>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include "Headers/Mesh.h"
#include "Headers/Shader.h"

Model::Model(std::vector<Mesh> meshes, std::vector<std::vector<Texture>> mesh_textures)
{
	meshes_ = meshes;
	mesh_textures_ = mesh_textures;
}

void Model::Draw(const Shader& shader)
{
	for (unsigned int i = 0; i < meshes_.size(); i++)
	{
		meshes_[i].Draw(shader, mesh_textures_[i]);
	}
}

void Model::DrawInstanced(const Shader& shader, std::vector<glm::mat4> instance_matrices)
{
    for (unsigned int i = 0; i < meshes_.size(); i++)
    {
        meshes_[i].DrawInstanced(shader, mesh_textures_[i], instance_matrices);
    }
}
