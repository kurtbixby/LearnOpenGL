#ifndef MODEL_H
#define MODEL_H

#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include "Mesh.h"
#include "Shader.h"

class Model
{
	public:
		//static void Init();

		Model(boost::filesystem::path path)
		{
			loadModel(path);
		}
		void Draw(Shader shader);
	private:
		static std::unordered_map<string, Texture> loaded_textures_;

		std::vector<Mesh> meshes_;
		std::string directory_;

		void loadModel(boost::filesystem::path path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType);

        static unsigned int TextureFromFile(const char* fileName, std::string directory);
};

#endif
