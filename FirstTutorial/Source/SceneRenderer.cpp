//
//  SceneRenderer.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/SceneRenderer.h"

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Headers/Object.h"
#include "Headers/UniformBlockBuffer.h"

#define LIGHT_DIMEN_H 50.0f
#define LIGHT_DIMEN_V 30.0f
#define LIGHT_NEAR_Z -30.0f
#define LIGHT_FAR_Z 70.0f

#define PNT_LGHT_NEAR 0.01f
#define PNT_LGHT_FAR 40.0f

#define SPT_LGHT_NEAR 0.01f
#define SPT_LGHT_FAR 60.0f

const GLuint MATRIX_BLOCK_IDX = 1;
const GLuint LIGHTING_BLOCK_IDX = 2;

const std::vector<std::string> SceneRenderer::deferredTextureNames_ = {
    "worldPosition",
    "worldNormal",
    "colorDiffuse",
    "colorSpec",
    "reflectDir",
    "reflectColor",
    "skyboxColor"
};

uint32_t SceneRenderer::DeferredFramebuffersNumber()
{
    return deferredTextureNames_.size();
}

// Unit vector for greyScale conversion/brightness measurement
const glm::vec3 SceneRenderer::bloomThreshold_ = glm::vec3(0.2126, 0.7152, 0.0722);

const glm::vec3 SceneRenderer::cubeShadMapDirections_[] = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f)
};

// Flip y-axis to account for texture UV mapping
const glm::vec3 SceneRenderer::cubeShadMapUps_[] = {
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};

SceneRenderer::SceneRenderer(Scene* scene, uint32_t shadowRes, ScreenRenderer* scrRenderer)
{
    scene_ = scene;
    scrRenderer_ = scrRenderer;
    modelLoader_ = ModelLoader(boost::filesystem::path("Resources").make_preferred());
    
    MakeShaders();
    MakeShadowMaps(shadowRes);
}

void SceneRenderer::MakeShaders()
{
    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTexturesInstanced_ShadMap_ManyLights.vert").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection_ShadMapSmooth_ManyLights_Bloom.frag").make_preferred();
    regularShader_ = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());
    
    boost::filesystem::path transparent_fragment_shader_path = boost::filesystem::path("Shaders/Transparency_Blended.frag").make_preferred();
    transparentShader_ = Shader(vertex_shader_path.string().c_str(), transparent_fragment_shader_path.string().c_str());
    
    boost::filesystem::path outline_fragment_shader_path = boost::filesystem::path("Shaders/Outline.frag").make_preferred();
    Shader outline_shader = Shader(vertex_shader_path.string().c_str(), outline_fragment_shader_path.string().c_str());
    
    boost::filesystem::path skybox_vertex_shader_path = boost::filesystem::path("Shaders/SkyboxBuff.vert").make_preferred();
    boost::filesystem::path skybox_fragment_shader_path = boost::filesystem::path("Shaders/Skybox.frag").make_preferred();
    skyboxShader_ = Shader(skybox_vertex_shader_path.string().c_str(), skybox_fragment_shader_path.string().c_str());
    
    boost::filesystem::path deff_skybox_fragment_shader_path = boost::filesystem::path("Shaders/Deferred_Skybox.frag").make_preferred();
    deferredSkyboxShader_ = Shader(skybox_vertex_shader_path.string().c_str(), deff_skybox_fragment_shader_path.string().c_str());
    
    boost::filesystem::path shadow_map_vertex_shader_path = boost::filesystem::path("Shaders/Dir_ShadowsInstanced.vert").make_preferred();
    boost::filesystem::path shadow_map_fragment_shader_path = boost::filesystem::path("Shaders/Dir_Shadows.frag").make_preferred();
    dirShadMapMakerShader_ = Shader(shadow_map_vertex_shader_path.string().c_str(), shadow_map_fragment_shader_path.string().c_str());
    
    boost::filesystem::path pnt_shadow_map_vert_shader_path = boost::filesystem::path("Shaders/Point_Shadows.vert").make_preferred();
    boost::filesystem::path pnt_shadow_map_geom_shader_path = boost::filesystem::path("Shaders/Point_Shadows.geom").make_preferred();
    boost::filesystem::path pnt_shadow_map_frag_shader_path = boost::filesystem::path("Shaders/Point_Shadows.frag").make_preferred();
    pntShadMapMakerShader_ = Shader(pnt_shadow_map_vert_shader_path.string().c_str(), pnt_shadow_map_geom_shader_path.string().c_str(), pnt_shadow_map_frag_shader_path.string().c_str());
    
    boost::filesystem::path spt_shadow_map_vert_shader_path = boost::filesystem::path("Shaders/Spot_Shadows.vert").make_preferred();
    boost::filesystem::path spt_shadow_map_frag_shader_path = boost::filesystem::path("Shaders/Spot_Shadows.frag").make_preferred();
    sptShadMapMakerShader_ = Shader(spt_shadow_map_vert_shader_path.string().c_str(), spt_shadow_map_frag_shader_path.string().c_str());

    boost::filesystem::path lights_vert_shader_path = boost::filesystem::path("Shaders/Lights.vert").make_preferred();
    boost::filesystem::path lights_frag_shader_path = boost::filesystem::path("Shaders/Lights_Bloom.frag").make_preferred();
    lightsShader_ = Shader(lights_vert_shader_path.string().c_str(), lights_frag_shader_path.string().c_str());
    
    
    boost::filesystem::path deferred_vert_shader_path = boost::filesystem::path("Shaders/Deferred_Vertex.vert").make_preferred();
    boost::filesystem::path deferred_gbuffer_frag_shader_path = boost::filesystem::path("Shaders/Deferred_BuffCreation.frag").make_preferred();
    deferredGBufferCreationShader_ = Shader(deferred_vert_shader_path.string().c_str(), deferred_gbuffer_frag_shader_path.string().c_str());
    
    boost::filesystem::path quad_vert_shader_path = boost::filesystem::path("Shaders/Screen.vert").make_preferred();
    boost::filesystem::path gbuffer_comp_frag_shader_path = boost::filesystem::path("Shaders/Deferred_BufferComposition.frag").make_preferred();
    deferredGBufferCompositionShader_ = Shader(quad_vert_shader_path.string().c_str(), gbuffer_comp_frag_shader_path.string().c_str());
}


void SceneRenderer::MakeShadowMaps(uint32_t shadowRes)
{
    std::vector<std::vector<Object>> regularDrawLists = scene_->RegularObjectsDrawLists();
    std::vector<std::vector<Object>> transparentDrawLists = scene_->TransparentObjectsDrawLists();
    SceneLighting lighting = scene_->ActiveLighting();
    
    glCullFace(GL_FRONT);
    
    Framebuffer shadowFramebuffer = Framebuffer(shadowRes, shadowRes, 1, false);
    shadowFramebuffer.SetViewPort();
    GenerateDirLightShadowMaps(lighting.DirLights(), regularDrawLists, shadowFramebuffer);
    GeneratePntLightShadowMaps(lighting.PntLights(), regularDrawLists, shadowFramebuffer);
    //    GenerateSpotShadowMaps(regularDrawLists, transparentDrawLists, shadowFramebuffer);
    
    glCullFace(GL_BACK);
    
    sentShadowMaps_ = false;
}

void SceneRenderer::GenerateDirLightShadowMaps(const std::vector<Light>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer)
{
    dirShadMapMakerShader_.Use();
    
    for (Light light : lights)
    {
        shadowFramebuffer.AddTextureAttachment(FBAttachment::Depth);
        shadowFramebuffer.Use();
        
        glm::vec3 position = glm::vec3(0.0f) - (20.0f * light.Direction());
        glm::mat4 lightView = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 1.0f, 0.0f));
        glm::mat4 lightProjection = glm::ortho(-LIGHT_DIMEN_H, LIGHT_DIMEN_H, -LIGHT_DIMEN_V, LIGHT_DIMEN_V, LIGHT_NEAR_Z, LIGHT_FAR_Z);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        dirShadMapMakerShader_.SetMatrix4fv("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto drawList : regularDrawLists)
        {
            if (drawList.front().Is2D_)
            {
                glDisable(GL_CULL_FACE);
            }
            RenderObjectsInstanced(drawList, dirShadMapMakerShader_);
            if (drawList.front().Is2D_)
            {
                glEnable(GL_CULL_FACE);
            }
        }
       
        GLuint shadowMap = shadowFramebuffer.RetrieveDepthBuffer().TargetName;
        shadowMaps_.dirLightShadowMaps.push_back(shadowMap);
        shadowMaps_.dirLightSpaceMats.push_back(lightSpaceMatrix);
    }
}

void SceneRenderer::GeneratePntLightShadowMaps(const std::vector<PointLight>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer)
{
    pntShadMapMakerShader_.Use();
    pntShadMapMakerShader_.SetFloat("farPlane", PNT_LGHT_FAR);
    
    float fov = glm::radians(90.0f);
    float aspect = shadowFramebuffer.AspectRatio();
    for (PointLight light : lights)
    {
        glm::vec3 position = light.Position();
        pntShadMapMakerShader_.SetVec3("lightPos", position.x, position.y, position.z);
        
        glm::mat4 lightProj = glm::perspective(fov, aspect, PNT_LGHT_NEAR, PNT_LGHT_FAR);
        for (int i = 0; i < 6; i++)
        {
            glm::mat4 lightView = glm::lookAt(position, position + cubeShadMapDirections_[i], cubeShadMapUps_[i]);
            glm::mat4 lightSpace = lightProj * lightView;
            
            std::string lightSpaceName = "lightSpace[";
            lightSpaceName.append(std::to_string(i)).append("]");
            pntShadMapMakerShader_.SetMatrix4fv(lightSpaceName, glm::value_ptr(lightSpace));
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
            RenderObjectsInstanced(drawList, pntShadMapMakerShader_);
            if (is2D)
            {
                glEnable(GL_CULL_FACE);
            }
        }
        
        GLuint cubemap = shadowFramebuffer.RetrieveDepthBuffer().TargetName;
        shadowMaps_.pntLightShadowMaps.push_back(cubemap);
    }
}

void SceneRenderer::RenderChanged()
{
    sentShadowMaps_ = false;
}

void SceneRenderer::Render_Forward(Framebuffer& mainBuffer)
{
    mainBuffer.SetViewPort();
    mainBuffer.Use();
    
    // Can wait until the draw calls
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // state setter
    glClearDepth(1.0f);
    glClearStencil(0x0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // state user
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    Camera cam = scene_->ActiveCamera();
    // Combine this into one function that returns on object?
    glm::mat4 projection = cam.GetProjection();
    glm::mat4 view = cam.MakeViewMat();
    
    const GLuint matrixBindIndex = MATRIX_BLOCK_IDX;
    UniformBlockBuffer<glm::mat4> matBuff = UniformBlockBuffer<glm::mat4>(2);
    matBuff.FillBuffer(projection);
    matBuff.FillBuffer(view);
    matBuff.BindToIndex(matrixBindIndex);
    
    Cubemap skybox = scene_->ActiveSkybox();
    
    SceneLighting lighting = scene_->ActiveLighting();
    const GLuint lightingBindIndex = LIGHTING_BLOCK_IDX;
    UniformBlockBuffer<SceneLighting> sceneLighting = UniformBlockBuffer<SceneLighting>(1);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lighting);
    
    // Regular rendering shader (uniform) setup
    regularShader_.Use();
    regularShader_.BindUniformBlock("Matrices", matrixBindIndex);
    regularShader_.BindUniformBlock("Lighting", lightingBindIndex);
    skybox.Activate(regularShader_);
    
    // Only have to do this when the shadow maps change
    if (scene_->LightingChanged() || !sentShadowMaps_)
    {
        SendShadowMapsToShader(regularShader_);
    }
    std::vector<std::vector<Object>> regularObjDrawLists = scene_->RegularObjectsDrawLists();
    if (regularObjDrawLists.size() > 0)
    {
        RenderDrawLists(regularObjDrawLists, regularShader_);
    }
    
    // Draw Lights
    lightsShader_.Use();
    lightsShader_.BindUniformBlock("Matrices", matrixBindIndex);
    lightsShader_.BindUniformBlock("Lighting", lightingBindIndex);
    lightsShader_.SetVec3("bloomThreshold", bloomThreshold_.r, bloomThreshold_.g, bloomThreshold_.b);
    DrawLights(lighting, lightsShader_);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    skyboxShader_.Use();
    skyboxShader_.BindUniformBlock("Matrices", matrixBindIndex);
    skybox.Draw(skyboxShader_);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
}

void SceneRenderer::SendShadowMapsToShader(Shader& shader)
{
#warning "REFACTOR THIS FOR MULTIPLE LIGHT SHADOW MAPS"
    shader.SetMatrix4fv("dirLightSpaceMatrices[0]", glm::value_ptr(shadowMaps_.dirLightSpaceMats.front()));
    shader.SetInt("dirLightShadowMaps[0]", DIR_SHAD_MAP_TEX_START);
    glActiveTexture(GL_TEXTURE0 + DIR_SHAD_MAP_TEX_START);
    glBindTexture(GL_TEXTURE_2D, shadowMaps_.dirLightShadowMaps[0]);
    
    shader.SetFloat("spotFarPlane", 40.0f);
    for (int i = 0; i < shadowMaps_.pntLightShadowMaps.size(); i++)
    {
        std::string samplerCube = "pointLightShadowMaps_" + std::to_string(i);
        shader.SetInt(samplerCube, PNT_SHAD_MAP_TEX_START + i);
        glActiveTexture(GL_TEXTURE0 + PNT_SHAD_MAP_TEX_START + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps_.pntLightShadowMaps[i]);
    }
    
    shader.SetVec3("bloomThreshold", bloomThreshold_.r, bloomThreshold_.g, bloomThreshold_.b);
    
    sentShadowMaps_ = true;
}

void SceneRenderer::DrawLights(SceneLighting& lighting, Shader& lightsShader)
{
    std::vector<Object> lightsList = std::vector<Object>();
    std::vector<PointLight> pointLights = lighting.PntLights();
    for (PointLight light : pointLights)
    {
        Object light_obj = Object(light.Position(), "Defaults/box", glm::vec3(0.5f), false);
        lightsList.push_back(light_obj);
    }
    RenderObjectsInstanced(lightsList, lightsShader);
}

void SceneRenderer::Render_Deferred(Framebuffer& compositeBuffer, Framebuffer& gBuffer)
{
    gBuffer.SetViewPort();
    gBuffer.Use();
    // Can wait until the draw calls
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // state setter
    glClearDepth(1.0f);
    glClearStencil(0x0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // state user
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    Camera cam = scene_->ActiveCamera();
    // Combine this into one function that returns on object?
    glm::mat4 projection = cam.GetProjection();
    glm::mat4 view = cam.MakeViewMat();
    glm::vec3 camPos = cam.GetPosition();
    
    const GLuint matrixBindIndex = MATRIX_BLOCK_IDX;
    UniformBlockBuffer<glm::mat4> matBuff = UniformBlockBuffer<glm::mat4>(2);
    matBuff.FillBuffer(projection);
    matBuff.FillBuffer(view);
    matBuff.BindToIndex(matrixBindIndex);
    
    SceneLighting lighting = scene_->ActiveLighting();
    const GLuint lightingBindIndex = LIGHTING_BLOCK_IDX;
    UniformBlockBuffer<SceneLighting> sceneLighting = UniformBlockBuffer<SceneLighting>(1);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lighting);
    
    deferredGBufferCreationShader_.Use();
    deferredGBufferCreationShader_.BindUniformBlock("Matrices", matrixBindIndex);
    deferredGBufferCreationShader_.SetVec3("camPosition", camPos.x, camPos.y, camPos.z);
    std::vector<std::vector<Object>> regularObjDrawLists = scene_->RegularObjectsDrawLists();
    if (regularObjDrawLists.size() > 0)
    {
        RenderDrawLists(regularObjDrawLists, deferredGBufferCreationShader_);
    }
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    deferredSkyboxShader_.Use();
    deferredSkyboxShader_.BindUniformBlock("Matrices", matrixBindIndex);
    Cubemap skybox = scene_->ActiveSkybox();
#warning Why does this draw call mess up the black skybox?
//    skybox.Draw(deferredSkyboxShader_);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    
    // COMPOSITING STAGE
    // Composite deferred textures to one
    deferredGBufferCompositionShader_.Use();
    deferredGBufferCompositionShader_.BindUniformBlock("Matrices", matrixBindIndex);
    deferredGBufferCompositionShader_.BindUniformBlock("Lighting", lightingBindIndex);
    deferredGBufferCompositionShader_.SetVec3("camPosition", camPos.x, camPos.y, camPos.z);
    skybox.Activate(deferredGBufferCompositionShader_);

    compositeBuffer.Use();
    
    CompositeDeferredRenderTextures(gBuffer, deferredGBufferCompositionShader_);
    
    // Can wait until the draw calls
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // state setter
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT); // state user
    glDisable(GL_DEPTH_TEST);
    scrRenderer_->DrawPostProcessScreenQuad();
    
    gBuffer.CopyAttachmentToFramebuffer(compositeBuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT);
    compositeBuffer.Use();
    glEnable(GL_DEPTH_TEST);
    // Draw lights
    
    // Draw transparent objects
    
    // Draw Lights
    lightsShader_.Use();
    lightsShader_.BindUniformBlock("Matrices", matrixBindIndex);
    lightsShader_.BindUniformBlock("Lighting", lightingBindIndex);
    lightsShader_.SetVec3("bloomThreshold", bloomThreshold_.r, bloomThreshold_.g, bloomThreshold_.b);
    DrawLights(lighting, lightsShader_);
}

void SceneRenderer::CompositeDeferredRenderTextures(Framebuffer& gBuffer, Shader& compositionShader)
{
    for (int i = 0; i < deferredTextureNames_.size(); i++)
    {
        GLuint textureUnitNumber = DEFERRED_RENDER_TEX_UNIT + i;
        GLuint textureName = gBuffer.RetrieveColorBuffer(i).TargetName;
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, textureName);
        compositionShader.SetInt(deferredTextureNames_[i], textureUnitNumber);
    }
    
    // Only have to do this when the shadow maps change
    if (scene_->LightingChanged() || !sentShadowMaps_)
    {
        SendShadowMapsToShader(compositionShader);
    }
    
    compositionShader.SetVec3("OutColor", 1.0f, 0.0f, 0.0f);
}

void SceneRenderer::RenderDrawLists(std::vector<std::vector<Object>> drawLists, Shader& shader)
{
    for (std::vector<Object> drawList : drawLists)
    {
        RenderObjectsInstanced(drawList, shader);
    }
}

void SceneRenderer::RenderObjectsInstanced(std::vector<Object> drawList, Shader& shader)
{
    if (drawList.size() < 1)
    {
        return;
    }
    
    Model model = modelLoader_.LoadModel(drawList.front().Model_);
    
    std::vector<glm::mat4> instanceMatrices = std::vector<glm::mat4>();
    
    // Prepare the transformation/states of each object
    for (const Object& obj : drawList)
    {
        instanceMatrices.push_back(obj.ModelMatrix());
    }
    
    model.DrawInstanced(shader, instanceMatrices);
}
