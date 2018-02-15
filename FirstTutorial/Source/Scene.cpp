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
#include "Model.h"
#include "Object.h"
#include "SceneGraph.h"
#include "Shader.h"
#include "Structs.h"
#include "UniformBlockBuffer.h"

Scene::Scene()
{
	graph_ = SceneGraph();
	skybox_ = Cubemap();
	cams_ = std::vector<Camera>();
	models_ = std::vector<Model>();
	shaders_ = std::vector<Shader>();
}

Scene::Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Model> models, std::vector<Shader> shaders, Cubemap skybox)
{
	graph_ = graph;
	skybox_ = skybox;
	cams_ = cams;
	models_ = models;
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

	const GLuint matrixBindIndex = 1;
	UniformBlockBuffer<glm::mat4> matBuff = UniformBlockBuffer<glm::mat4>(2);
	matBuff.FillBuffer(projection);
	matBuff.FillBuffer(view);
	matBuff.BindToIndex(matrixBindIndex);

	Shader& standardShader = shaders_[0];
	Shader& transparentShader = shaders_[1];
	Shader& outlineShader = shaders_[2];
	Shader& skyboxShader = shaders_[3];

	// For partially transparent
	// sort by Z position relative to camera in direction of "front", back to front
	// project object position onto infinite "front" vector

	// Implementation: subtract camera position and sort by dot product of resultant and cam.direction
	// Optimization: sort only during movement
	graph_.UseCamera(cam);
	std::vector<Object> objects = graph_.RelevantObjects();

	std::vector<Object> transparent = std::vector<Object>();
	std::vector<Object> stenciled = std::vector<Object>();
	std::vector<Object> regular = std::vector<Object>();

	for (int i = 0; i < objects.size(); i++)
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
			regular.push_back(object);
		}
	}

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	skyboxShader.Use();

	glActiveTexture(GL_TEXTURE0 + 2);
	skyboxShader.SetInt("skybox", 2);

	skyboxShader.BindUniformBlock("Matrices", matrixBindIndex);

	skybox_.Draw();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	std::vector<Light> lights = graph_.RelevantLights();
	std::vector<PointLight> pointLights = graph_.RelevantPointLights();
	std::vector<SpotLight> spotLights = graph_.RelevantSpotLights();
    SceneLighting lighting = graph_.RelevantLighting();

	unsigned int DIR_LIGHTS = lights.size();
	unsigned int POINT_LIGHTS = pointLights.size();
	unsigned int SPOT_LIGHTS = spotLights.size();

    const GLuint lightingBindIndex = 2;
    UniformBlockBuffer<SceneLighting> sceneLighting = UniformBlockBuffer<SceneLighting>(1);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lighting);
    
    standardShader.Use();
    standardShader.BindUniformBlock("Matrices", matrixBindIndex);
    standardShader.BindUniformBlock("Lighting", lightingBindIndex);

	standardShader.SetInt("skybox", 20);
	glActiveTexture(GL_TEXTURE0 + 20);
	skybox_.Activate();

	RenderObjects(regular, standardShader);

	skybox_.Deactivate();
	glActiveTexture(GL_TEXTURE0);

    transparentShader.Use();
    transparentShader.BindUniformBlock("Matrices", matrixBindIndex);
    transparentShader.BindUniformBlock("Lighting", lightingBindIndex);
//    SendLights(transparentShader, DIR_LIGHTS, lights, POINT_LIGHTS, pointLights, SPOT_LIGHTS, spotLights);
    RenderObjects(transparent, transparentShader);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // First pass
    standardShader.Use();
    RenderObjects(stenciled, standardShader);

    glClear(GL_DEPTH_BUFFER_BIT);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    // Outline pass
    outlineShader.Use();
    outlineShader.BindUniformBlock("Matrices", matrixBindIndex);
    RenderObjects(stenciled, outlineShader);

    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
}

void Scene::TakeInput(const Input& input)
{
    cams_[0].TakeInput(input.cameraInput);
}

void Scene::RenderObjects(const vector<Object>& objects, const Shader& shader)
{
	for (int i = 0; i < objects.size(); i++)
	{
		const Object& object = objects[i];

		Model model = models_[object.Model_];
		glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), object.Transform_);
		modelMat = glm::scale(modelMat, object.Scale_);
		shader.SetMatrix4fv("model", glm::value_ptr(modelMat));
		object.Draw(shader, model);
	}
}

void Scene::SendLights(Shader& shader, int DIR_LIGHTS, std::vector<Light>& lights, int POINT_LIGHTS, std::vector<PointLight>& pointLights, int SPOT_LIGHTS, std::vector<SpotLight>& spotLights)
{
	shader.SetInt("DIR_LIGHTS", std::min(MAX_DIR_LIGHTS, DIR_LIGHTS));
	shader.SetInt("POINT_LIGHTS", std::min(MAX_POINT_LIGHTS, POINT_LIGHTS));
	shader.SetInt("SPOT_LIGHTS", std::min(MAX_SPOT_LIGHTS, SPOT_LIGHTS));

	for (int i = 0; i < DIR_LIGHTS; i++)
	{
		Light* light = &lights[i];
		shader.AddDirectionLight(*light, i);
	}

	for (int i = 0; i < POINT_LIGHTS; i++)
	{
		PointLight* light = &pointLights[i];
		shader.AddPointLight(*light, i);
	}

	for (int i = 0; i < SPOT_LIGHTS; i++)
	{
		SpotLight* light = &spotLights[i];
		shader.AddSpotLight(*light, i);
	}
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
