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
	models_ = std::vector<Model>();
    lightShadowMaps_ = std::vector<uint32_t>();
    pointLightShadowMaps_ = std::vector<uint32_t>();
    lightSpaceMats_ = std::vector<glm::mat4>();
    
    standardShader_ = Shader();
    transparentShader_ = Shader();
    outlineShader_ = Shader();
    skyboxShader_ = Shader();
    geometryShader_ = Shader();
    dirShadowMapShader_ = Shader();
    pointShadowMapShader_ = Shader();
    spotShadowMapShader_ = Shader();
    lightsShader_ = Shader();
    
    inUseDefaultShader_ = standardShader_;
}

Scene::Scene(SceneGraph graph, std::vector<Camera> cams, std::vector<Model> models, std::vector<Shader> shaders, Cubemap skybox)
{
	graph_ = graph;
	skybox_ = skybox;
	cams_ = cams;
	models_ = models;
    
    lightShadowMaps_ = std::vector<uint32_t>();
    pointLightShadowMaps_ = std::vector<uint32_t>();
    lightSpaceMats_ = std::vector<glm::mat4>();
    
    standardShader_ = shaders[0];
    transparentShader_ = shaders[1];
    outlineShader_ = shaders[2];
    skyboxShader_ = shaders[3];
    geometryShader_ = shaders[4];
    altLightShader_ = shaders[5];
    dirShadowMapShader_ = shaders[6];
    pointShadowMapShader_ = shaders[7];
    spotShadowMapShader_ = shaders[8];
    lightsShader_ = shaders[9];
    
    inUseDefaultShader_ = standardShader_;
}

void Scene::GenerateShadowMaps(Framebuffer& shadowFramebuffer)
{
    // Attaches own depth attachment
    
    glCullFace(GL_FRONT);
    // Generate shadow maps and attach them to lights
    
    std::vector<Object> transparentObjects = std::vector<Object>();
    std::vector<Object> regularObjects = std::vector<Object>();
    for (Object obj : graph_.RelevantObjects())
    {
        if (obj.IsTransparent_)
        {
            transparentObjects.push_back(obj);
        }
        else
        {
            regularObjects.push_back(obj);
        }
    }
    
    std::vector<std::vector<Object>> regularDrawLists;
    if (regularObjects.size() > 0)
    {
        regularDrawLists = CreateRegularDrawLists(regularObjects);
    }
    std::vector<std::vector<Object>> transparentDrawLists;
    if (transparentObjects.size() > 0)
    {
        transparentDrawLists = CreateTransparentDrawLists(transparentObjects);
    }
    
    shadowFramebuffer.SetViewPort();
    
    GenerateDirectionalShadowMaps(regularDrawLists, transparentDrawLists, shadowFramebuffer);
    
    GeneratePointShadowMaps(regularDrawLists, transparentDrawLists, shadowFramebuffer);

//    GenerateSpotShadowMaps(regularDrawLists, transparentDrawLists, shadowFramebuffer);
    
    glCullFace(GL_BACK);
}

void Scene::GenerateDirectionalShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer)
{
    dirShadowMapShader_.Use();
    for (Light light : graph_.RelevantLights())
    {
        // Makes a new texture attachment
        // Memory leak?
        // No, just delete the textures eventually (?)
        shadowFramebuffer.AddTextureAttachment(FBAttachment::Depth);
        shadowFramebuffer.Use();
        glm::vec3 position = glm::vec3(0.0f) - (20.0f * light.Direction());
        glm::mat4 lightView = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 1.0f, 0.0f));
        glm::mat4 lightProjection = glm::ortho(-LIGHT_DIMEN_H, LIGHT_DIMEN_H, -LIGHT_DIMEN_V, LIGHT_DIMEN_V, LIGHT_NEAR_Z, LIGHT_FAR_Z);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        lightSpaceMats_.push_back(lightSpaceMatrix);
        dirShadowMapShader_.SetMatrix4fv("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto drawList : regularDrawLists)
        {
            if (drawList.front().Is2D_)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, dirShadowMapShader_);
            if (drawList.front().Is2D_)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        for (auto drawList : transparentDrawLists)
        {
            if (drawList.front().Is2D_)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, dirShadowMapShader_);
            if (drawList.front().Is2D_)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        
        // Attach framebuffer to Light
        auto depthBuffer = shadowFramebuffer.RetrieveDepthBuffer();
        lightShadowMaps_.push_back(depthBuffer.TargetName);
    }
}

void Scene::GeneratePointShadowMaps(const std::vector<std::vector<Object>> &regularDrawLists, const std::vector<std::vector<Object>> &transparentDrawLists, Framebuffer& shadowFramebuffer)
{
    pointShadowMapShader_.Use();
    
    glm::vec3 directions[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
    };
    
    // Flip y-axis to account for texture UV mapping
    glm::vec3 up[] = {
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    };
    
    pointShadowMapShader_.SetFloat("farPlane", PNT_LGHT_FAR);
    for (PointLight light : graph_.RelevantPointLights())
    {
        // Make vector to store all target names
        std::vector<glm::mat4> lightSpaceMatrices = std::vector<glm::mat4>();
        
        float fov = glm::radians(90.0f);
        float aspect = shadowFramebuffer.AspectRatio();
        glm::mat4 lightProj = glm::perspective(fov, aspect, PNT_LGHT_NEAR, PNT_LGHT_FAR);
        glm::vec3 position = light.Position();
        for (int i = 0; i < 6; i++)
        {
            glm::mat4 lightView = glm::lookAt(position, position + directions[i], up[i]);
            glm::mat4 lightSpace = lightProj * lightView;
            lightSpaceMatrices.push_back(lightSpace);
        }
        
        pointShadowMapShader_.SetVec3("lightPos", position.x, position.y, position.z);
        for (int i = 0; i < 6; i++)
        {
            std::string lightSpaceName = "lightSpace[";
            lightSpaceName.append(std::to_string(i)).append("]");
            pointShadowMapShader_.SetMatrix4fv(lightSpaceName, glm::value_ptr(lightSpaceMatrices[i]));
        }
        
        shadowFramebuffer.AddCubemapAttachment(FBAttachment::Depth);
        shadowFramebuffer.Use();
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto drawList : regularDrawLists)
        {
            bool is2D = drawList.front().Is2D_;
            if (is2D)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, pointShadowMapShader_);
            if (is2D)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        for (auto drawList : transparentDrawLists)
        {
            bool is2D = drawList.front().Is2D_;
            if (is2D)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, pointShadowMapShader_);
            if (is2D)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        
        auto depthBuffer = shadowFramebuffer.RetrieveDepthBuffer();
        pointLightShadowMaps_.push_back(depthBuffer.TargetName);
    }
}

void Scene::GenerateSpotShadowMaps(const std::vector<std::vector<Object> > &regularDrawLists, const std::vector<std::vector<Object> > &transparentDrawLists, Framebuffer &shadowFramebuffer)
{
    spotShadowMapShader_.Use();
    spotShadowMapShader_.SetFloat("farPlane", SPT_LGHT_FAR);
    spotShadowMapShader_.SetVec2("center", shadowFramebuffer.Center().x, shadowFramebuffer.Center().y);
    
    for (SpotLight light : graph_.RelevantSpotLights())
    {
        float fov = light.LightAngle();
        float aspect = shadowFramebuffer.AspectRatio();
        glm::mat4 lightProj = glm::perspective(fov, aspect, SPT_LGHT_NEAR, SPT_LGHT_FAR);
        glm::vec3 position = light.Position();
        glm::vec3 direction = light.Direction();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // arbitrary up in y-axis
        glm::mat4 lightView = glm::lookAt(position, position + direction, up);
        spotLightSpaceMats_.push_back(lightProj * lightView);
        
        shadowFramebuffer.AddTextureAttachment(FBAttachment::Depth);
        shadowFramebuffer.Use();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto drawList : regularDrawLists)
        {
            bool is2D = drawList.front().Is2D_;
            if (is2D)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, spotShadowMapShader_);
            if (is2D)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        for (auto drawList : transparentDrawLists)
        {
            bool is2D = drawList.front().Is2D_;
            if (is2D)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, spotShadowMapShader_);
            if (is2D)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        
        auto depthBuffer = shadowFramebuffer.RetrieveDepthBuffer();
        spotLightShadowMaps_.push_back(depthBuffer.TargetName);
    }
}

void Scene::Render()
{
	// render section of main loop
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // state setter
    glClearDepth(1.0f);
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
    skyboxShader_.Use();
    
    skyboxShader_.SetInt("skybox", SKYBOX_TEX_UNIT);
    skyboxShader_.BindUniformBlock("Matrices", matrixBindIndex);
    
    skybox_.Draw();
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

	std::vector<Light> lights = graph_.RelevantLights();
	std::vector<PointLight> pointLights = graph_.RelevantPointLights();
	std::vector<SpotLight> spotLights = graph_.RelevantSpotLights();
    SceneLighting lighting = graph_.RelevantLighting();

    const GLuint lightingBindIndex = 2;
    UniformBlockBuffer<SceneLighting> sceneLighting = UniformBlockBuffer<SceneLighting>(1);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lighting);
    
    if (regular.size() > 0)
    {
        std::vector<std::vector<Object>> reg_draw_list_inst = CreateRegularDrawLists(regular);
        
        inUseDefaultShader_.Use();
        inUseDefaultShader_.BindUniformBlock("Matrices", matrixBindIndex);
        inUseDefaultShader_.BindUniformBlock("Lighting", lightingBindIndex);

        inUseDefaultShader_.SetInt("skybox", SKYBOX_TEX_UNIT);
        glActiveTexture(GL_TEXTURE0 + SKYBOX_TEX_UNIT);
        skybox_.Activate();

#warning "REFACTOR THIS FOR MULTIPLE LIGHT SHADOW MAPS"
        inUseDefaultShader_.SetMatrix4fv("dirLightSpaceMatrices[0]", glm::value_ptr(lightSpaceMats_.front()));
        inUseDefaultShader_.SetInt("dirLightShadowMaps[0]", DIR_SHAD_MAP_TEX_START);
        glActiveTexture(GL_TEXTURE0 + DIR_SHAD_MAP_TEX_START);
        glBindTexture(GL_TEXTURE_2D, lightShadowMaps_[0]);
        
        inUseDefaultShader_.SetFloat("spotFarPlane", 40.0f);
        for (int i = 0; i < pointLightShadowMaps_.size(); i++)
        {
            std::string samplerCube = "pointLightShadowMaps_" + std::to_string(i);
            inUseDefaultShader_.SetInt(samplerCube, PNT_SHAD_MAP_TEX_START + i);
            glActiveTexture(GL_TEXTURE0 + PNT_SHAD_MAP_TEX_START + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightShadowMaps_[i]);
        }
        
//        inUseDefaultShader_.SetMatrix4fv("spotLightSpaceMatrices[0]", glm::value_ptr(spotLightSpaceMats_.front()));
//        inUseDefaultShader_.SetInt("spotLightShadowMaps[0]", SPT_SHAD_MAP_TEX_START);
//        glActiveTexture(GL_TEXTURE0 + SPT_SHAD_MAP_TEX_START);
//        glBindTexture(GL_TEXTURE_2D, spotLightShadowMaps_[0]);
        
//        inUseDefaultShader_.SetInt("pointLightShadowMaps_1", PNT_SHAD_MAP_TEX_START);
//        glActiveTexture(GL_TEXTURE0 + PNT_SHAD_MAP_TEX_START + 1);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightShadowMaps_[1]);
        
        // Unit vector for greyScale conversion/brightness measurement
        glm::vec3 bloomThreshold = glm::vec3(0.2126, 0.7152, 0.0722);
        inUseDefaultShader_.SetVec3("bloomThreshold", bloomThreshold.r, bloomThreshold.g, bloomThreshold.b);
        
        for (std::vector<Object> draw_list : reg_draw_list_inst)
        {
//            RenderObjects(draw_list, standardShader);
            RenderObjectsInstanced(draw_list, inUseDefaultShader_);
        }
        
        lightsShader_.Use();
        lightsShader_.BindUniformBlock("Matrices", matrixBindIndex);
        lightsShader_.BindUniformBlock("Lighting", lightingBindIndex);
        lightsShader_.SetVec3("bloomThreshold", bloomThreshold.r, bloomThreshold.g, bloomThreshold.b);
        std::vector<Object> light_list = std::vector<Object>();
        // Draw Lights
        for (PointLight light : pointLights)
        {
            Object light_obj = Object(light.Position(), 1, glm::vec3(0.5f), false);
            light_list.push_back(light_obj);
        }
        RenderObjectsInstanced(light_list, lightsShader_);

        skybox_.Deactivate();
        glActiveTexture(GL_TEXTURE0);
        
//        bonusShader.Use();
//        bonusShader.BindUniformBlock("Matrices", matrixBindIndex);
//        RenderObjects(regular, bonusShader);
    }
    
    if (transparent.size() > 0)
    {
        std::vector<std::vector<Object>> trans_draw_list_inst = CreateTransparentDrawLists(transparent);

        transparentShader_.Use();
        transparentShader_.BindUniformBlock("Matrices", matrixBindIndex);
        transparentShader_.BindUniformBlock("Lighting", lightingBindIndex);
        
        for(std::vector<Object> draw_list : trans_draw_list_inst)
        {
//            RenderObjects(draw_list, transparentShader);
            RenderObjectsInstanced(draw_list, transparentShader_);
        }
    }

    if (stenciled.size() > 0)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        
        // First pass
        inUseDefaultShader_.Use();
        RenderObjects(stenciled, standardShader_);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        
        // Outline pass
        outlineShader_.Use();
        outlineShader_.BindUniformBlock("Matrices", matrixBindIndex);
        RenderObjects(stenciled, outlineShader_);
        
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
    }
}

void Scene::RenderSimple()
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
    
    graph_.UseCamera(cam);
    std::vector<Object> objects = graph_.RelevantObjects();
    std::vector<Light> lights = graph_.RelevantLights();
    
    const GLuint lightingBindIndex = 2;
    UniformBlockBuffer<Light> sceneLighting = UniformBlockBuffer<Light>(2);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lights[0]);
    sceneLighting.FillBuffer(lights[1]);
    
    inUseDefaultShader_.Use();
    inUseDefaultShader_.BindUniformBlock("Matrices", matrixBindIndex);
    inUseDefaultShader_.BindUniformBlock("Lighting", lightingBindIndex);
    
    for (Object obj : objects)
    {
        Model m = ModelForId(obj.Model_);
        
        glm::mat4 modelMatrix = obj.ModelMatrix();
        glm::mat4 thisModMat = glm::translate(modelMatrix, glm::vec3(0.0f, -0.5f, 0.0f));
        inUseDefaultShader_.SetMatrix4fv("model", glm::value_ptr(thisModMat));
        
        m.Draw(inUseDefaultShader_);
    }
}

void Scene::TakeInput(const Input& input)
{
    cams_[0].TakeInput(input.CameraInput());
    if (input.BlinnLighting())
    {
        inUseDefaultShader_ = altLightShader_;
    }
    else
    {
        inUseDefaultShader_ = standardShader_;
    }
}

//void Scene::Simulate()
//{
//
//}

Model Scene::ModelForId(uint32_t model_id) const
{
    return models_[model_id];
}

std::vector<std::vector<Object>> Scene::CreateRegularDrawLists(const vector<Object>& objects)
{
    std::vector<std::vector<Object>> reg_draw_list_inst = std::vector<std::vector<Object>>();
    std::unordered_map<uint32_t, uint32_t> reg_model_id_obj_list_map = std::unordered_map<uint32_t, uint32_t>();
    
    // Group regular
    for (Object obj : objects)
    {
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
    uint32_t previous_model = objects.front().Model_;
    std::vector<Object> current_list = std::vector<Object>();
    
    // Group transparent
    for (Object obj : objects)
    {
        if (obj.Model_ == previous_model)
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

void Scene::RenderObjects(const vector<Object>& objects, const Shader& shader)
{
	for (int i = 0; i < objects.size(); i++)
	{
		const Object& object = objects[i];

		Model model = models_[object.Model_];
        glm::mat4 modelMat = object.ModelMatrix();
//        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), object.Transform_);
//        modelMat = glm::scale(modelMat, object.Scale_);
		shader.SetMatrix4fv("model", glm::value_ptr(modelMat));
		object.Draw(shader, model);
	}
}

void Scene::RenderObjectsInstanced(const vector<Object>& draw_list, const Shader& shader)
{
    if (draw_list.size() < 1)
    {
        return;
    }
    
    Model model = ModelForId(draw_list.front().Model_);
    
    std::vector<glm::mat4> instance_matrices = std::vector<glm::mat4>();
    
    // Prepare the transformation/states of each object
    for (const Object& obj : draw_list)
    {
        instance_matrices.push_back(obj.ModelMatrix());
    }
    
    model.DrawInstanced(shader, instance_matrices);
}

//void Scene::SendLights(Shader& shader, int DIR_LIGHTS, std::vector<Light>& lights, int POINT_LIGHTS, std::vector<PointLight>& pointLights, int SPOT_LIGHTS, std::vector<SpotLight>& spotLights)
//{
//    shader.SetInt("DIR_LIGHTS", std::min(MAX_DIR_LIGHTS, DIR_LIGHTS));
//    shader.SetInt("POINT_LIGHTS", std::min(MAX_POINT_LIGHTS, POINT_LIGHTS));
//    shader.SetInt("SPOT_LIGHTS", std::min(MAX_SPOT_LIGHTS, SPOT_LIGHTS));
//
//    for (int i = 0; i < DIR_LIGHTS; i++)
//    {
//        Light* light = &lights[i];
//        shader.AddDirectionLight(*light, i);
//    }
//
//    for (int i = 0; i < POINT_LIGHTS; i++)
//    {
//        PointLight* light = &pointLights[i];
//        shader.AddPointLight(*light, i);
//    }
//
//    for (int i = 0; i < SPOT_LIGHTS; i++)
//    {
//        SpotLight* light = &spotLights[i];
//        shader.AddSpotLight(*light, i);
//    }
//}

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
