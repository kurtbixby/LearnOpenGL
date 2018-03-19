#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <vector>

#include "Structs.h"

#define SKYBOX_TEX_UNIT 15

class Cubemap
{
public:
	Cubemap();
    Cubemap(uint32_t cubemapName);
	Cubemap(std::vector<std::string> textureFiles);

	void Activate();
	void Deactivate();
	void Draw();

private:
	unsigned int vao_;
	unsigned int texId_;
	const static float Vertices_[];
};

#endif
