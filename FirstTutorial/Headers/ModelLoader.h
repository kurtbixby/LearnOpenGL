#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assimp/scene.h>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <unordered_map>

#include "ModelTexture.h"
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
	Model LoadModel(std::string file);

private:
    struct aiSceneWrapper {
        std::string relative_dir;
        const aiScene* scene;
    };
    
	static unordered_map<string, ModelTexture> loaded_textures_;
    static unordered_map<string, Model> loaded_models_;

    const static std::string defaultModelsFolder_;
	std::string directory_;

    Model loadDefaultModel(std::string file);
	void processNode(aiNode* node, aiSceneWrapper wrapped_scene, std::vector<Mesh>& meshes, std::vector<std::vector<ModelTexture>>& mesh_textures);
	Mesh processMesh(aiMesh* mesh, aiSceneWrapper wrapped_scene, std::vector<ModelTexture>& textures);
    std::vector<ModelTexture> loadMaterialTextures(std::string relative_dir, aiMaterial* mat, aiTextureType type, ModelTextureType texType);
};

#endif
