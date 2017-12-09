#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Mesh.h"
#include "Shader.h"

class Object
{
	public:
		glm::vec3 Transform_;
		int Mesh_;
		glm::vec3 Scale_;
		bool Outlined_;
		bool Is2D_;

		Object() : Object(glm::vec3(1.0f), -1, 1.0f, false) {};
		Object(const glm::vec3 transform, const int mesh, const float scale, const bool outline, const bool is2D = false)
		{
			Transform_ = transform;
			Mesh_ = mesh;
			Scale_ = glm::vec3(scale);
			Outlined_ = outline;
			Is2D_ = is2D;
		}

		void Draw(Shader shader, Mesh& mesh)
		{
			if (Is2D_)
			{
				glDisable(GL_CULL_FACE);
			}
			mesh.Draw(shader);
			if (Is2D_)
			{
				glEnable(GL_CULL_FACE);
			}
		}
};

#endif
