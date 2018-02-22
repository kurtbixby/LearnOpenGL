#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assimp/scene.h>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <unordered_map>

#include "Mesh.h"
#include "Model.h"
#include "Structs.h"

enum class DefaultModel
{
	Box,
	Plane,
	Quad
};

class ModelLoader
{
public:
	ModelLoader();
	ModelLoader(boost::filesystem::path path);
	Model loadModel(std::string file);
//    Model loadDefault(DefaultModel model);

private:
	static unordered_map<string, Texture> loaded_textures_;

	std::string directory_;

	void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, std::vector<std::vector<Texture>>& mesh_textures);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<Texture>& textures);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType);
};

#endif
