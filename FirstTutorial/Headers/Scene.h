#ifndef SCENE_H
#define SCENE_H

#define MAX_CAMERAS 1

#include "Headers/Camera.h"
#include "Headers/Cubemap.h"
#include "Headers/Framebuffer.h"
#include "Headers/Model.h"
#include "Headers/Object.h"
#include "Headers/SceneGraph.h"
#include "Headers/Structs.h"
#include "Headers/Shader.h"

class Scene
{
public:
	Scene();
	Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Model> models, std::vector<Shader> shaders, Cubemap skybox);

	void LoadCameras(std::vector<Camera>);
	void LoadModels(std::vector<Model>);
	void LoadShaders(std::vector<Shader>);
    
    void GenerateShadowMaps(Framebuffer& shadowFramebuffer);
	void Render();
	void TakeInput(const Input& input);
	
private:
	SceneGraph graph_;
	Cubemap skybox_;

	std::vector<Camera> cams_;
	std::vector<Model> models_;
    
    std::vector<uint32_t> lightShadowMaps_;
    std::vector<glm::mat4> lightSpaceMats_;
    std::vector<uint32_t> pointLightShadowMaps_;
    std::vector<glm::mat4> pointLightSpaceMats_;
    
    Shader inUseDefaultShader_;
    
    Shader standardShader_;
    Shader altLightShader_;
    Shader transparentShader_;
    Shader outlineShader_;
    Shader skyboxShader_;
    Shader geometryShader_;
    Shader dirShadowMapShader_;
    Shader pointShadowMapShader_;
    Shader spotShadowMapShader_;
    
    void GenerateDirectionalShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);
    
    void GeneratePointShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);
    
    void GenerateSpotShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);

    Model ModelForId(uint32_t model_id) const;
    std::vector<std::vector<Object>> CreateRegularDrawLists(const vector<Object>& objects);
    std::vector<std::vector<Object>> CreateTransparentDrawLists(const vector<Object>& objects);
	void RenderObjects(const vector<Object>& objects, const Shader& shader);
    void RenderObjectsInstanced(const vector<Object>& draw_list, const Shader& shader);
//    void SendLights(Shader& shader, int DIR_LIGHTS, std::vector<Light>& lights, int POINT_LIGHTS, std::vector<PointLight>& pointLights, int SPOT_LIGHTS, std::vector<SpotLight>& spotLights);
};

#endif
