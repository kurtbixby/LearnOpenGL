#include "Headers/Primitives.h"

#include <vector>

#include "Headers/Mesh.h"
#include "Headers/Model.h"
#include "Headers/Structs.h"

Model create_plane()
{
	std::vector<Mesh> meshes = std::vector<Mesh>();
	meshes.push_back(create_plane_mesh());

	std::vector<std::vector<Texture>> mesh_textures = std::vector<std::vector<Texture>>();
	std::vector<Texture> textures = std::vector<Texture>();
	textures.push_back(load_texture("Resources", "awesomeface.png", TextureType::Diffuse));
	mesh_textures.push_back(textures);

	return Model(meshes, mesh_textures);
}

Model create_box()
{
	std::vector<Mesh> meshes = std::vector<Mesh>();
	meshes.push_back(create_box_mesh());

	std::vector<std::vector<Texture>> mesh_textures = std::vector<std::vector<Texture>>();
	std::vector<Texture> textures = std::vector<Texture>();
	textures.push_back(load_texture("Resources", "container2_diffuse.png", TextureType::Diffuse));
	textures.push_back(load_texture("Resources", "container2_specular.png", TextureType::Specular));
	mesh_textures.push_back(textures);

	return Model(meshes, mesh_textures);
}

Model create_quad()
{
	std::vector<Mesh> meshes = std::vector<Mesh>();
	meshes.push_back(create_quad_mesh());

	std::vector<std::vector<Texture>> mesh_textures = std::vector<std::vector<Texture>>();
	std::vector<Texture> textures = std::vector<Texture>();
	textures.push_back(load_texture("Resources", "blending_transparent_window.png", TextureType::Diffuse));
	mesh_textures.push_back(textures);

	return Model(meshes, mesh_textures);
}

Mesh create_plane_mesh()
{
	float vertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		-5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f,
		-5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,

		5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,
		5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f
	};

	int length = sizeof(vertices) / sizeof(vertices[0]);

	std::vector<Vertex> vertexes;

	for (int i = 0; i < length; i += 8)
	{
		glm::vec3 pos = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
		glm::vec3 norm = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
		glm::vec2 coords = glm::vec2(vertices[i + 6], vertices[i + 7]);

		vertexes.push_back(create_vertex(pos, norm, coords));
	}

	std::vector<unsigned int> indices = {
		0, 1, 2,
		3, 4, 5
	};

	return Mesh(vertexes, indices);
}

Mesh create_box_mesh()
{
	float vertices[] = {
		// positions          // normals           // texture coords
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f
	};

	int length = sizeof(vertices) / sizeof(vertices[0]);

	std::vector<Vertex> vertexes;

	for (int i = 0; i < length; i += 8)
	{
		glm::vec3 pos = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
		glm::vec3 norm = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
		glm::vec2 coords = glm::vec2(vertices[i + 6], vertices[i + 7]);

		vertexes.push_back(create_vertex(pos, norm, coords));
	}

	std::vector<unsigned int> indices = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35
	};

	return Mesh(vertexes, indices);	
}

Mesh create_quad_mesh()
{
	std::vector<Vertex> vertices = {
		create_vertex(-0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f),
		create_vertex(0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f),
		create_vertex(0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f),
		create_vertex(0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f),
		create_vertex(-0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f),
		create_vertex(-0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f)
	};

	std::vector<unsigned int> indices = {
		0, 1, 2,
		3, 4, 5
	};

	return Mesh(vertices, indices);	
}
