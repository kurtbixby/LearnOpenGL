#define NOMINMAX

#include "Scene.h"

#include <algorithm>
#include <glad/glad.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Cubemap.h"
#include "Lights.h"
#include "Mesh.h"
#include "Object.h"
#include "SceneGraph.h"
#include "Shader.h"
#include "Structs.h"

Scene::Scene()
{
	graph_ = SceneGraph();
	skybox_ = Cubemap();
	cams_ = std::vector<Camera>();
	meshes_ = std::vector<Mesh>();
	shaders_ = std::vector<Shader>();
}

Scene::Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Mesh> meshes, std::vector<Shader> shaders, Cubemap skybox)
{
	graph_ = graph;
	skybox_ = skybox;
	cams_ = cams;
	meshes_ = meshes;
	shaders_ = shaders;
}

void Scene::Render()
{
	// render section of main loop
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // state setter
	// glClearDepth(1.0f); // glClearDepth(0.0f); is default clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user

    Camera& cam = cams_[0];

    glm::mat4 projection = cam.GetProjection();
    glm::mat4 view = cam.MakeViewMat();

	Shader& standardShader = shaders_[0];
	Shader& outlineShader = shaders_[1];
	Shader& skyboxShader = shaders_[2];

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	skyboxShader.Use();

	skyboxShader.SetMatrix4fv("projection", glm::value_ptr(projection));
	skyboxShader.SetMatrix4fv("view", glm::value_ptr(glm::mat4(glm::mat3(view))));

	skybox_.Draw();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	standardShader.Use();
    standardShader.SetMatrix4fv("projection", glm::value_ptr(projection));
    standardShader.SetMatrix4fv("view", glm::value_ptr(view));

    std::vector<Light> lights = graph_.RelevantLights();
    std::vector<PointLight> pointLights = graph_.RelevantPointLights();
    std::vector<SpotLight> spotLights = graph_.RelevantSpotLights();

    // Lights to shader section
    int DIR_LIGHTS = lights.size();
    int POINT_LIGHTS = pointLights.size();
    int SPOT_LIGHTS = spotLights.size();

    standardShader.SetInt("DIR_LIGHTS", std::min(MAX_DIR_LIGHTS, DIR_LIGHTS));
    standardShader.SetInt("POINT_LIGHTS", std::min(MAX_POINT_LIGHTS, POINT_LIGHTS));
    standardShader.SetInt("SPOT_LIGHTS", std::min(MAX_SPOT_LIGHTS, SPOT_LIGHTS));

    for (int i = 0; i < DIR_LIGHTS; i++)
    {
        Light* light = &lights[i];
        standardShader.AddDirectionLight(*light, i);
    }

    for (int i = 0; i < POINT_LIGHTS; i++)
    {
        PointLight* light = &pointLights[i];
        standardShader.AddPointLight(*light, i);
    }

    for (int i = 0; i < SPOT_LIGHTS; i++)
    {
        SpotLight* light = &spotLights[i];
        standardShader.AddSpotLight(*light, i);
    }

    // End Lights to shader section


    // For partially transparent
    // sort by Z position relative to camera in direction of "front", back to front
    // project object position onto infinite "front" vector

    // Implementation: subtract camera position and sort by dot product of resultant and cam.direction
    // Optimization: sort only during movement
    graph_.UseCamera(cam);
    std::vector<Object> objects = graph_.RelevantObjects();

    std::vector<Object> transparent = std::vector<Object>();
    std::vector<Object> stenciled = std::vector<Object>();

    for(int i = 0; i < objects.size(); i++)
    {
        Object& object = objects[i];
        if (object.IsTransparent_)
        {
            transparent.push_back(object);
            continue;
        }
        else if (object.Outlined_)
        {
            stenciled.push_back(object);
            continue;
        }
        else
        {
            Mesh mesh = meshes_[object.Mesh_];
            glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
            model = glm::scale(model, object.Scale_);
            standardShader.SetMatrix4fv("model", glm::value_ptr(model));
            object.Draw(standardShader, mesh);
        }
    }

    for (int i = 0; i < transparent.size(); i++)
    {
        Object& object = transparent[i];

        Mesh mesh = meshes_[object.Mesh_];
        glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
        model = glm::scale(model, object.Scale_);
        standardShader.SetMatrix4fv("model", glm::value_ptr(model));
        object.Draw(standardShader, mesh);
    }

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    
    // First pass
    for (int i = 0; i < stenciled.size(); i++)
    {
        Object& object = stenciled[i];

        Mesh mesh = meshes_[object.Mesh_];
        glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
        model = glm::scale(model, object.Scale_);
        standardShader.SetMatrix4fv("model", glm::value_ptr(model));
        mesh.Draw(standardShader);
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    // Outline pass
    outlineShader.Use();
    outlineShader.SetMatrix4fv("projection", glm::value_ptr(projection));
    outlineShader.SetMatrix4fv("view", glm::value_ptr(view));
    for (int i = 0; i < stenciled.size(); i++)
    {
        Object& object = stenciled[i];

        Mesh mesh = meshes_[object.Mesh_];
        glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
        model = glm::scale(model, object.Scale_ * 1.1f);
        outlineShader.SetMatrix4fv("model", glm::value_ptr(model));
        mesh.Draw(outlineShader);
    }
    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
}

void Scene::TakeInput(const Input& input)
{
    cams_[0].TakeInput(input.cameraInput);
}

// Crysis path
// standard_shader.SetMatrix4fv("model", glm::value_ptr(model));

// boost::filesystem::path model_path = boost::filesystem::path("Resources/nanosuit/nanosuit.obj").make_preferred();
// string model_path_string = model_path.string();

// auto models_iterator = loaded_models_.find(model_path_string);
// if (models_iterator != loaded_models_.end())
// {
// 	cout << model_path_string << " already loaded" << endl;
// 	models_iterator->second.Draw(standard_shader);
// }
// else
// {
// 	cout << model_path_string << " not loaded" << endl;
// 	Model model = Model(model_path);
// 	loaded_models_.emplace(model_path_string, model);
// 	model.Draw(standard_shader);
// }
