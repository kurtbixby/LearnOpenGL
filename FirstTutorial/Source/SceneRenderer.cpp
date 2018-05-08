//
//  SceneRenderer.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/SceneRenderer.h"

#include <random>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Headers/TextureCreator.h"
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
    "reflectColor"
};

const std::vector<std::string> SceneRenderer::deferredSSAOTextureNames_ = {
    "viewPosition",
    "viewNormal"
};

uint32_t SceneRenderer::DeferredFramebuffersNumber()
{
    return deferredTextureNames_.size() + deferredSSAOTextureNames_.size();
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

SceneRenderer::SceneRenderer(Scene* scene, ScreenRenderer* scrRenderer, RenderConfig* renderConfig)
{
    scene_ = scene;
    scrRenderer_ = scrRenderer;
    modelLoader_ = ModelLoader(boost::filesystem::path("Resources").make_preferred());
    renderConfig_ = renderConfig;
    ssaoEnabled_ = renderConfig_->SSAOEnabled();
    
    MakeShaders();
    MakeFramebuffers(renderConfig_->RenderWidth(), renderConfig_->RenderHeight());
    MakeShadowMaps(renderConfig_->ShadowMapRes());
    
    SSAO_Setup();
}

void SceneRenderer::MakeFramebuffers(uint32_t renderWidth, uint32_t renderHeight, uint32_t samples)
{
    forwardBuffer_ = Framebuffer(renderWidth, renderHeight, samples);
    forwardBuffer_.AddTextureAttachment(FBAttachment::ColorHDR);
    forwardBuffer_.AddTextureAttachment(FBAttachment::ColorHDR);
    forwardBuffer_.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    uint32_t bufferClearBits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    forwardBuffer_.SetBufferClear(bufferClearBits, clearColor);
    
    gBuffer_ = Framebuffer(renderWidth, renderHeight);
    for (int i = 0; i < SceneRenderer::DeferredFramebuffersNumber(); i++)
    {
        gBuffer_.AddTextureAttachment(FBAttachment::ColorHDR);
    }
    gBuffer_.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    bufferClearBits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    gBuffer_.SetBufferClear(bufferClearBits);
    
    deferredCompBuffer_ = Framebuffer(renderWidth, renderHeight);
    deferredCompBuffer_.AddTextureAttachment(FBAttachment::ColorHDR);
    deferredCompBuffer_.AddTextureAttachment(FBAttachment::ColorHDR);
    deferredCompBuffer_.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    bufferClearBits = GL_COLOR_BUFFER_BIT;
    clearColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    deferredCompBuffer_.SetBufferClear(bufferClearBits, clearColor);
    
    ssaoBuffer_ = Framebuffer(renderWidth, renderHeight);
    // Refactor this to allow single channel textures
    ssaoBuffer_.AddTextureAttachment(FBAttachment::Color);
    bufferClearBits = GL_COLOR_BUFFER_BIT;
    ssaoBuffer_.SetBufferClear(bufferClearBits, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    
    ssaoBlurBuffer_ = Framebuffer(renderWidth, renderHeight);
    ssaoBlurBuffer_.AddTextureAttachment(FBAttachment::Color);
    bufferClearBits = GL_COLOR_BUFFER_BIT;
    ssaoBlurBuffer_.SetBufferClear(bufferClearBits, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
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
    
    boost::filesystem::path defer_skybox_fragment_shader_path = boost::filesystem::path("Shaders/Deferred_Skybox.frag").make_preferred();
    deferredSkyboxShader_ = Shader(skybox_vertex_shader_path.string().c_str(), defer_skybox_fragment_shader_path.string().c_str());
    
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
    boost::filesystem::path gbuffer_comp_frag_shader_path = boost::filesystem::path("Shaders/Deferred_BufferComposition_SSAO.frag").make_preferred();
    deferredGBufferCompositionShader_ = Shader(quad_vert_shader_path.string().c_str(), gbuffer_comp_frag_shader_path.string().c_str());
    
    boost::filesystem::path ssao_frag_shader_path = boost::filesystem::path("Shaders/Deferred_SSAO.frag").make_preferred();
    deferredSSAOShader_ = Shader(quad_vert_shader_path.string().c_str(), ssao_frag_shader_path.string().c_str());
    
    boost::filesystem::path ssao_blur_frag_shader_path = boost::filesystem::path("Shaders/SSAO_Blur.frag").make_preferred();
    ssaoBlurShader_ = Shader(quad_vert_shader_path.string().c_str(), ssao_blur_frag_shader_path.string().c_str());
}


void SceneRenderer::MakeShadowMaps(uint32_t shadowRes)
{
    std::vector<std::vector<Object>> regularDrawLists = scene_->RegularObjectsDrawLists();
    std::vector<std::vector<Object>> transparentDrawLists = scene_->TransparentObjectsDrawLists();
    SceneLighting lighting = scene_->ActiveLighting();
    
    glCullFace(GL_FRONT);
    
    Framebuffer shadowFramebuffer = Framebuffer(shadowRes, shadowRes, 1, false);
    shadowFramebuffer.SetBufferClear(GL_DEPTH_BUFFER_BIT);
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
        glm::vec3 position = glm::vec3(0.0f) - (20.0f * light.Direction());
        glm::mat4 lightView = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 1.0f, 0.0f));
        glm::mat4 lightProjection = glm::ortho(-LIGHT_DIMEN_H, LIGHT_DIMEN_H, -LIGHT_DIMEN_V, LIGHT_DIMEN_V, LIGHT_NEAR_Z, LIGHT_FAR_Z);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        dirShadMapMakerShader_.SetMatrix4fv("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        
        shadowFramebuffer.AddTextureAttachment(FBAttachment::DepthHiRes);
        shadowFramebuffer.Use();
        shadowFramebuffer.Clear();
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
        
        shadowFramebuffer.AddCubemapAttachment(FBAttachment::DepthHiRes);
        shadowFramebuffer.Use();
        shadowFramebuffer.Clear();
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

const Framebuffer* SceneRenderer::Render_Forward()
{
    Framebuffer& mainBuffer = forwardBuffer_;
    mainBuffer.SetViewPort();
    mainBuffer.Use();
    mainBuffer.Clear();
    
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
    
    return &forwardBuffer_;
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

const Framebuffer* SceneRenderer::Render_Deferred()
{
    Framebuffer& gBuffer = gBuffer_;
    gBuffer.SetViewPort();
    gBuffer.Use();
    gBuffer.Clear();
    
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
    
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    
    if (ssaoEnabled_)
    {
        // SSAO
        // Make/use SSAO framebuffer
        // Render out SSAO
        // Buffer not cleared
        ssaoBuffer_.Use();
        ssaoBuffer_.Clear();
        deferredSSAOShader_.Use();
        deferredSSAOShader_.BindUniformBlock("Matrices", matrixBindIndex);
        SSAO_SendViewTextures(deferredSSAOShader_, gBuffer);
        scrRenderer_->DrawPostProcessScreenQuad();
        ssaoBlurBuffer_.Use();
        ssaoBlurBuffer_.Clear();
        ssaoBlurShader_.Use();
        ssaoBlurShader_.SetInt("blurSize", 4);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, ssaoBuffer_.RetrieveColorBuffer(0).TargetName);
        ssaoBlurShader_.SetInt("ssaoTexture", 1);
        scrRenderer_->DrawPostProcessScreenQuad();
    }
    
    // COMPOSITING STAGE
    // Composite deferred textures to one
    deferredGBufferCompositionShader_.Use();
    deferredGBufferCompositionShader_.BindUniformBlock("Matrices", matrixBindIndex);
    deferredGBufferCompositionShader_.BindUniformBlock("Lighting", lightingBindIndex);
    deferredGBufferCompositionShader_.SetVec3("camPosition", camPos.x, camPos.y, camPos.z);
    Cubemap skybox = scene_->ActiveSkybox();
    skybox.Activate(deferredGBufferCompositionShader_);

    Framebuffer& compositeBuffer = deferredCompBuffer_;
    compositeBuffer.Use();
    //    // Can wait until the draw calls
    //    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // state setter
    //    glClear(GL_COLOR_BUFFER_BIT); // state user
    compositeBuffer.Clear();
    
    CompositeDeferredRenderTextures(deferredGBufferCompositionShader_, gBuffer, ssaoBlurBuffer_);
    scrRenderer_->DrawPostProcessScreenQuad();
    
    gBuffer.CopyAttachmentToFramebuffer(compositeBuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT);
    compositeBuffer.Use();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    
    // Draw transparent objects
    
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
    deferredSkyboxShader_.Use();
    deferredSkyboxShader_.BindUniformBlock("Matrices", matrixBindIndex);
    skybox.Draw(deferredSkyboxShader_);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    
    return &compositeBuffer;
}

void SceneRenderer::CompositeDeferredRenderTextures(Shader& compositionShader, Framebuffer& gBuffer, Framebuffer& ssaoBuffer)
{
    for (int i = 0; i < deferredTextureNames_.size(); i++)
    {
        GLuint textureUnitNumber = DEFERRED_RENDER_TEX_UNIT + i;
        GLuint textureName = gBuffer.RetrieveColorBuffer(i).TargetName;
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, textureName);
        compositionShader.SetInt(deferredTextureNames_[i], textureUnitNumber);
    }
    
    GLuint ssaoTexUnitNumber = DEFERRED_RENDER_TEX_UNIT + deferredTextureNames_.size();
    GLuint ssaoTextureName = ssaoBuffer.RetrieveColorBuffer(0).TargetName;
    glActiveTexture(GL_TEXTURE0 + ssaoTexUnitNumber);
    glBindTexture(GL_TEXTURE_2D, ssaoTextureName);
    compositionShader.SetInt("ssaoTexture", ssaoTexUnitNumber);
    
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

void SceneRenderer::RenderObjectsInstanced(std::vector<Object> drawList, Shader& shader, GLuint fillMode)
{
    if (drawList.size() < 1)
    {
        return;
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, fillMode);
    
    Model model = modelLoader_.LoadModel(drawList.front().Model_);
    
    std::vector<glm::mat4> instanceMatrices = std::vector<glm::mat4>();
    
    // Prepare the transformation/states of each object
    for (const Object& obj : drawList)
    {
        instanceMatrices.push_back(obj.ModelMatrix());
    }
    
    model.DrawInstanced(shader, instanceMatrices);
}

void SceneRenderer::SSAO_Setup()
{
    // Generate fragment offsets to sample from
    std::vector<glm::vec3> sampleOffsets = SSAO_SampleOffsets(64);
    // Generate noise texture for better results (optional)
    GLuint noiseTex = SSAO_NoiseTexture();
    deferredSSAOShader_.Use();
    SSAO_SendConstantData(deferredSSAOShader_, sampleOffsets, noiseTex);
}

std::vector<glm::vec3> SceneRenderer::SSAO_SampleOffsets(uint32_t sampleNumber)
{
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> sampleOffsets;
    for (uint32_t i = 0; i < sampleNumber; i++)
    {
        // Generate hemisphere of samples
        glm::vec3 sample = glm::vec3(distribution(generator) * 2.0 - 1.0,
                                     distribution(generator) * 2.0 - 1.0,
                                     distribution(generator));
        sample = glm::normalize(sample);
        float radiusScale = (float) i / (float) sampleNumber;
        radiusScale = lerp(0.1, 1.0, radiusScale * radiusScale);
        sampleOffsets.push_back(sample * radiusScale);
    }
    
    // Make this into a texture?
    return sampleOffsets;
}

float SceneRenderer::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

GLuint SceneRenderer::SSAO_NoiseTexture()
{
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 4; i++)
    {
        glm::vec3 noise(
                        distribution(generator) * 2.0 - 1.0,
                        distribution(generator) * 2.0 - 1.0,
                        0.0f);
        noise = glm::normalize(noise);
        ssaoNoise.push_back(noise);
    }
    
    TextureOptions options;
    options.TexFormat = GL_RGB;
    options.SourceDataFormat = GL_RGB;
    options.SourceDataType = GL_FLOAT;
    options.MinFilter = GL_NEAREST;
    options.MagFilter = GL_NEAREST;
    options.WrapType = GL_REPEAT;
    
    GLuint noiseTex = TextureCreator::CreateTexture(options, 4, 4, 1, &ssaoNoise[0]);
    return noiseTex;
}

void SceneRenderer::SSAO_SendConstantData(Shader& shader, std::vector<glm::vec3>& sampleOffsets, GLuint noiseTex)
{
    uint32_t noiseNum = DEFERRED_RENDER_TEX_UNIT + DeferredFramebuffersNumber();
    glActiveTexture(GL_TEXTURE0 + noiseNum);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    shader.SetInt("rotationNoise", noiseNum);
    
    shader.SetInt("numSamples", sampleOffsets.size());
    for (uint32_t i = 0; i < sampleOffsets.size(); i++)
    {
        glm::vec3 sampleOffset = sampleOffsets[i];
        shader.SetVec3("sampleOffsets[" + std::to_string(i) + "]", sampleOffset.x, sampleOffset.y, sampleOffset.z);
    }
    
    float ssaoRadius = 0.75f;
    shader.SetFloat("ssaoRadius", ssaoRadius);
}

void SceneRenderer::SSAO_SendViewTextures(Shader& shader, Framebuffer& gBuffer)
{
    uint32_t defTexs = deferredTextureNames_.size();
    uint32_t ssaoTexNum = defTexs + DEFERRED_RENDER_TEX_UNIT;
    for (uint32_t i = 0; i < deferredSSAOTextureNames_.size(); i++)
    {
        GLuint defTexId = gBuffer.RetrieveColorBuffer(defTexs + i).TargetName;
        GLuint texNum = ssaoTexNum + i;
        glActiveTexture(GL_TEXTURE0 + texNum);
        glBindTexture(GL_TEXTURE_2D, defTexId);
        shader.SetInt(deferredSSAOTextureNames_[i], texNum);
    }
}
