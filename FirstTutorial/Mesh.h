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
	private:
		unsigned int vao_, vbo_, ebo_;
		void initializeMesh();
};

#endif
