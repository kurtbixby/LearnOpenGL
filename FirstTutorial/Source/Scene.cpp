#define NOMINMAX

#include "Headers/Scene.h"

#include <algorithm>
#include <glad/glad.h>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Headers/Camera.h"
#include "Headers/Cubemap.h"
#include "Headers/Framebuffer.h"
#include "Headers/Lights.h"
#include "Headers/Model.h"
#include "Headers/ModelLoader.h"
#include "Headers/Object.h"
#include "Headers/SceneGraph.h"
#include "Headers/Shader.h"
#include "Headers/Structs.h"
#include "Headers/Timer.h"
#include "Headers/UniformBlockBuffer.h"

// Arbitrary to cover the entire scene based on one camera position
#define LIGHT_DIMEN_H 50.0f
#define LIGHT_DIMEN_V 30.0f
#define LIGHT_NEAR_Z -30.0f
#define LIGHT_FAR_Z 70.0f

#define PNT_LGHT_NEAR 0.01f
#define PNT_LGHT_FAR 40.0f

#define SPT_LGHT_NEAR 0.01f
#define SPT_LGHT_FAR 60.0f

Scene::Scene()
{
	graph_ = SceneGraph();
	skybox_ = Cubemap();
	cams_ = std::vector<Camera>();
    activeCameraIndex_ = 0;
    dirtyLighting_ = true;
    
    modelLoader_ = ModelLoader();
}

Scene::Scene(SceneGraph graph, std::vector<Camera> cams, ModelLoader modelLoader, std::vector<Shader> shaders, Cubemap skybox)
{
	graph_ = graph;
	skybox_ = skybox;
	cams_ = cams;
    activeCameraIndex_ = 0;
    dirtyLighting_ = true;
    
    modelLoader_ = modelLoader;
}

void Scene::TakeInput(const Input& input)
{
    cams_[0].TakeInput(input.CameraInput());
}

void Scene::Simulate()
{
    graph_.UseCamera(cams_[activeCameraIndex_]);
}

Camera Scene::ActiveCamera()
{
    return cams_[activeCameraIndex_];
}

SceneLighting Scene::ActiveLighting()
{
    dirtyLighting_ = false;
    return graph_.RelevantLighting();
}

bool Scene::LightingChanged()
{
    return dirtyLighting_;
}

Cubemap Scene::ActiveSkybox()
{
    return skybox_;
}

std::vector<std::vector<Object>> Scene::RegularObjectsDrawLists()
{
    std::vector<Object> objects = graph_.RelevantObjects();
    return CreateRegularDrawLists(objects);
}

std::vector<std::vector<Object>> Scene::TransparentObjectsDrawLists()
{
    std::vector<Object> objects = graph_.RelevantObjects();
    return CreateTransparentDrawLists(objects);
}

std::vector<std::vector<Object>> Scene::CreateRegularDrawLists(const vector<Object>& objects)
{
    std::vector<std::vector<Object>> reg_draw_list_inst = std::vector<std::vector<Object>>();
    std::unordered_map<std::string, uint32_t> reg_model_id_obj_list_map = std::unordered_map<std::string, uint32_t>();
    
    // Group regular
    for (Object obj : objects)
    {
        if (obj.IsTransparent_)
        {
            continue;
        }
        auto cur_model_obj_list = reg_model_id_obj_list_map.find(obj.Model_);
        if (cur_model_obj_list != reg_model_id_obj_list_map.end())
        {
            uint32_t cur_model = cur_model_obj_list->second;
            reg_draw_list_inst[cur_model].push_back(obj);
        }
        else
        {
            uint32_t model_list_index = reg_draw_list_inst.size();
            reg_draw_list_inst.push_back(std::vector<Object>());
            reg_draw_list_inst[model_list_index].push_back(obj);
            reg_model_id_obj_list_map.emplace(obj.Model_, model_list_index);
        }
    }
    
    return reg_draw_list_inst;
}

std::vector<std::vector<Object>> Scene::CreateTransparentDrawLists(const vector<Object>& objects)
{
    std::vector<std::vector<Object>> trans_draw_list_inst = std::vector<std::vector<Object>>();
    std::string previous_model = objects.front().Model_;
    std::vector<Object> current_list = std::vector<Object>();
    
    // Group transparent
    for (Object obj : objects)
    {
        if (!obj.IsTransparent_)
        {
            continue;
        }
        if (obj.Model_.compare(previous_model) == 0)
        {
            current_list.push_back(obj);
        }
        else
        {
            trans_draw_list_inst.push_back(current_list);
            current_list = std::vector<Object>();
            current_list.push_back(obj);
            previous_model = obj.Model_;
        }
    }
    trans_draw_list_inst.push_back(current_list);
    
    return trans_draw_list_inst;
}
