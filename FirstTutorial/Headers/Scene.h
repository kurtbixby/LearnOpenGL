#ifndef SCENE_H
#define SCENE_H

#define MAX_CAMERAS 1

#include "Headers/Camera.h"
#include "Headers/Cubemap.h"
#include "Headers/Framebuffer.h"
#include "Headers/Model.h"
#include "Headers/ModelLoader.h"
#include "Headers/Object.h"
#include "Headers/SceneGraph.h"
#include "Headers/Structs.h"
#include "Headers/Shader.h"

class Scene
{
public:
	Scene();
	Scene(SceneGraph graph, std::vector<Camera> cams, ModelLoader modelLoader, std::vector<Shader> shaders, Cubemap skybox);

//    void LoadCameras(std::vector<Camera>);
//    void LoadModels(std::vector<Model>);
//    void LoadShaders(std::vector<Shader>);
    
    void TakeInput(const Input& input);
    
    std::vector<std::vector<Object>> RegularObjectsDrawLists();
    std::vector<std::vector<Object>> TransparentObjectsDrawLists();
    
    Camera ActiveCamera();
    SceneLighting ActiveLighting();
    
    // Move to SceneRenderer
    void GenerateShadowMaps(Framebuffer& shadowFramebuffer);
	void Render();
    void RenderDeferred();
    void RenderSimple();
	//
private:
	SceneGraph graph_;
	Cubemap skybox_;

	std::vector<Camera> cams_;
    uint32_t activeCameraIndex_;
    
    // Move to SceneRenderer
    std::vector<uint32_t> lightShadowMaps_;
    std::vector<glm::mat4> lightSpaceMats_;
    std::vector<uint32_t> pointLightShadowMaps_;
    std::vector<uint32_t> spotLightShadowMaps_;
    std::vector<glm::mat4> spotLightSpaceMats_;
    //
    
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
    Shader lightsShader_;
    
    ModelLoader modelLoader_;
    Model ModelForId(std::string modelName);
    
    void DrawRegularObjectsDeferred(GLuint lightingBindIndex, GLuint matrixBindIndex, const std::vector<Object> &regular);
    
    std::vector<std::vector<Object>> CreateRegularDrawLists(const vector<Object>& objects);
    // Might be a pointless function. Probably have to draw each transparent object individually.
    std::vector<std::vector<Object>> CreateTransparentDrawLists(const vector<Object>& objects);
    
    // Move to SceneRenderer
    void GenerateDirectionalShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);
    void GeneratePointShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);
    void GenerateSpotShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer);

	void RenderObjects(const vector<Object>& objects, const Shader& shader);
    void RenderObjectsInstanced(const vector<Object>& draw_list, const Shader& shader);
    //
};

#endif
