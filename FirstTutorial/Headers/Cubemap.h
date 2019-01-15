#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <string>
#include <vector>

#include "Headers/Shader.h"

class Cubemap
{
public:
	Cubemap();
    Cubemap(uint32_t cubemapName);
	Cubemap(std::vector<std::string> textureFiles);

	void Activate(Shader& shader, std::string cubemapName = "skybox");
	void Deactivate();
	void Draw(Shader& shader);

private:
	unsigned int vao_;
	unsigned int texId_;
	const static float Vertices_[];
};

#endif
