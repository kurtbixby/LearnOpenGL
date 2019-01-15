#ifndef PRIMITIVES_LOADER_H
#define PRIMITIVES_LOADER_H

#include <string>
#include <unordered_map>

#include "Mesh.h"
#include "Model.h"

class PrimitivesLoader {
    enum PrimitivesNames {
        Plane,
        Box,
        Quad,
        BrickWall,
        Sphere
    };
    
    const static std::unordered_map<std::string, PrimitivesNames> modelNameMap_;
    
public:
    static Model LoadPrimitive(std::string primitiveFile);
    
private:
    static Model create_plane();
    static Model create_box();
    static Model create_quad();
    static Model create_brick_wall();
    static Model create_sphere();
    
    static Mesh create_plane_mesh();
    static Mesh create_box_mesh();
    static Mesh create_quad_mesh();
    static Mesh create_sphere_mesh();
};

#endif
