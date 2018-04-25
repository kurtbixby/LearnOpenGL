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

const std::vector<std::string> SceneRenderer::deferredTextureNames_ = {
    "worldPosition",
    "worldNormal",
    "colorDiffuse",
    "colorSpec",
    "reflectDir",
    "reflectColor"
};

SceneRenderer::SceneRenderer()
{
    modelLoader_ = ModelLoader(boost::filesystem::path("Resources").make_preferred());
}

std::vector<std::vector<GLuint>> SceneRenderer::MakeShadowMaps(Scene& scene)
{
#warning Unimplemented
    //
    return std::vector<std::vector<GLuint>>();
}

GLuint SceneRenderer::RenderScene_Forward(Scene &scene)
{
#warning Implement later
    return 0;
}

GLuint SceneRenderer::RenderScene_Deferred(Scene &scene, uint32_t width_, uint32_t height_)
{
    // Can wait until the draw calls
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // state setter
    glClearDepth(1.0f);
    // glClearDepth(1.0f); // glClearDepth(0.0f); is default clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user
    
    Camera cam = scene.ActiveCamera();
    // Combine this into one function that returns on object?
    glm::mat4 projection = cam.GetProjection();
    glm::mat4 view = cam.MakeViewMat();
    
    const GLuint matrixBindIndex = 1;
    UniformBlockBuffer<glm::mat4> matBuff = UniformBlockBuffer<glm::mat4>(2);
    matBuff.FillBuffer(projection);
    matBuff.FillBuffer(view);
    matBuff.BindToIndex(matrixBindIndex);
    
    SceneLighting lighting = scene.ActiveLighting();
    const GLuint lightingBindIndex = 2;
    UniformBlockBuffer<SceneLighting> sceneLighting = UniformBlockBuffer<SceneLighting>(1);
    sceneLighting.BindToIndex(lightingBindIndex);
    sceneLighting.FillBuffer(lighting);
    
    // Use deferred texture creation shader
    // Bind buffer block indices
    Framebuffer deferredTextures = Framebuffer(width_, height_);
    // Could do for (std::string name : deferredTextureNames_), but this uses less memory(?)
    for (int i = 0; i < deferredTextureNames_.size(); i++)
    {
        deferredTextures.AddTextureAttachment(FBAttachment::ColorHDR);
    }
    deferredTextures.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    
    deferredTextureCreationShader_.Use();
    deferredTextureCreationShader_.BindUniformBlock("Matrices", matrixBindIndex);
    deferredTextureCreationShader_.BindUniformBlock("Lighting", lightingBindIndex);
    std::vector<std::vector<Object>> regularObjDrawLists = scene.RegularObjectsDrawLists();
    if (regularObjDrawLists.size() > 0)
    {
        RenderDrawLists(regularObjDrawLists, deferredTextureCreationShader_);
    }
    
    // Composite deferred textures to one
    deferredTextureCompositionShader_.Use();
    deferredTextureCreationShader_.BindUniformBlock("Matrices", matrixBindIndex);
    deferredTextureCreationShader_.BindUniformBlock("Lighting", lightingBindIndex);
    
    Framebuffer compositeBuffer = Framebuffer(width_, height_);
    deferredTextures.AddTextureAttachment(FBAttachment::ColorHDR);
    deferredTextures.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    compositeBuffer.Use();
    
    // Draw skybox during composite
    CompositeDeferredRenderTextures(deferredTextures, deferredTextureCompositionShader_);
    
    // Draw lights
    
    // Draw transparent objects
    
    return 0;
}

void SceneRenderer::CompositeDeferredRenderTextures(Framebuffer& deferredBuffer, Shader& compositionShader)
{
    for (int i = 0; i < deferredTextureNames_.size(); i++)
    {
        GLuint textureUnitNumber = GL_TEXTURE0 + DEFERRED_RENDER_TEX_UNIT + i;
        GLuint textureName = deferredBuffer.RetrieveColorBuffer(i).TargetName;
        glActiveTexture(textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, textureName);
        compositionShader.SetInt(deferredTextureNames_[i], textureUnitNumber);
    }
    
//#warning Refactor this for a dynamic # of shadow maps. (Requires code gen?)
//    compositionShader.SetMatrix4fv("dirLightSpaceMatrices[0]", glm::value_ptr(lightSpaceMats_.front()));
//    compositionShader.SetInt("dirLightShadowMaps[0]", DIR_SHAD_MAP_TEX_START);
//    glActiveTexture(GL_TEXTURE0 + DIR_SHAD_MAP_TEX_START);
//    glBindTexture(GL_TEXTURE_2D, lightShadowMaps_[0]);
//
//    compositionShader.SetFloat("spotFarPlane", 40.0f);
//    for (int i = 0; i < pointLightShadowMaps_.size(); i++)
//    {
//        std::string samplerCube = "pointLightShadowMaps_" + std::to_string(i);
//        glActiveTexture(GL_TEXTURE0 + PNT_SHAD_MAP_TEX_START + i);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightShadowMaps_[i]);
//        compositionShader.SetInt(samplerCube, PNT_SHAD_MAP_TEX_START + i);
//    }
    
    // Skybox rendering
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
