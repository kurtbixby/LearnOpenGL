#ifndef SCENE_H
#define SCENE_H

#define MAX_CAMERAS 1

#include "Camera.h"
#include "Cubemap.h"
#include "Object.h"
#include "SceneGraph.h"
#include "Structs.h"
#include "Shader.h"

class Scene
{
public:
	Scene();
	Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Mesh> meshes, std::vector<Shader> shaders, Cubemap skybox);

	void LoadCameras(std::vector<Camera>);
	void LoadMeshes(std::vector<Mesh>);
	void LoadShaders(std::vector<Shader>);

	void Render();
	void TakeInput(const Input& input);
	
private:
	SceneGraph graph_;
	Cubemap skybox_;

	std::vector<Camera> cams_;
	std::vector<Mesh> meshes_;
	std::vector<Shader> shaders_;
};

#endif
