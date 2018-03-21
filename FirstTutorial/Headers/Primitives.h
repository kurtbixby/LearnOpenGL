#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "Model.h"

Model create_plane();
Model create_box();
Model create_quad();
Model create_brick_wall();

Mesh create_plane_mesh();
Mesh create_box_mesh();
Mesh create_quad_mesh();

#endif
