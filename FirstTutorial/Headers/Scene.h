#ifndef SCENE_H
#define SCENE_H

#define MAX_CAMERAS 1

#include "Camera.h"
#include "Cubemap.h"
#include "Model.h"
#include "Object.h"
#include "SceneGraph.h"
#include "Structs.h"
#include "Shader.h"

class Scene
{
public:
	Scene();
	Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Model> models, std::vector<Shader> shaders, Cubemap skybox);

	void LoadCameras(std::vector<Camera>);
	void LoadModels(std::vector<Model>);
	void LoadShaders(std::vector<Shader>);

	void Render();
	void TakeInput(const Input& input);
	
private:
	SceneGraph graph_;
	Cubemap skybox_;

	std::vector<Camera> cams_;
	std::vector<Model> models_;
	std::vector<Shader> shaders_;

	void RenderObjects(const vector<Object>& objects, const Shader& shader);
	void SendLights(Shader& shader, int DIR_LIGHTS, std::vector<Light>& lights, int POINT_LIGHTS, std::vector<PointLight>& pointLights, int SPOT_LIGHTS, std::vector<SpotLight>& spotLights);
};

#endif
