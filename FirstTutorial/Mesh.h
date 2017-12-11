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
		vector<Texture> textures_;

		Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
		void Draw(Shader shader);
	private:
		unsigned int vao_, vbo_, ebo_;
		void initializeMesh();
};

#endif
