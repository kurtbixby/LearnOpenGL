#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <vector>

#include "Structs.h"

class Cubemap
{
public:
	Cubemap();
	Cubemap(std::vector<std::string> textureFiles);

	void Activate();
	void Draw();

private:
	unsigned int vao_;
	unsigned int texId_;
	const static float Vertices_[];
};

#endif
