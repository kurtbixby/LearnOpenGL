#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "Mesh.h"
#include "Shader.h"

class Model
{
	public:
		Model(std::vector<Mesh> meshes, std::vector<std::vector<Texture>> mesh_textures);
		void Draw(Shader shader);

	private:
		std::vector<Mesh> meshes_;
		std::vector<std::vector<Texture>> mesh_textures_;
};

#endif
