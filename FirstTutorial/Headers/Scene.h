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
    
    void TakeInput(const Input& input);
    void Simulate();
    
    std::vector<std::vector<Object>> RegularObjectsDrawLists();
    std::vector<std::vector<Object>> TransparentObjectsDrawLists();
    
    Camera ActiveCamera();
    SceneLighting ActiveLighting();
    Cubemap ActiveSkybox();
    
private:
	SceneGraph graph_;
	Cubemap skybox_;

	std::vector<Camera> cams_;
    uint32_t activeCameraIndex_;
    
    ModelLoader modelLoader_;
    
    std::vector<std::vector<Object>> CreateRegularDrawLists(const vector<Object>& objects);
    // Might be a pointless function. Probably have to draw each transparent object individually.
    std::vector<std::vector<Object>> CreateTransparentDrawLists(const vector<Object>& objects);
};

#endif
