//
//  Renderer.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/19/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/Renderer.h"

#include "Headers/RenderConfig.h"
#include "Headers/SceneRenderer.h"
#include "Headers/ScreenRenderer.h"
#include "Headers/Shader.h"

#define PINGBUFFER 0
#define PONGBUFFER 1
#define COMPBUFFER 2

Renderer::Renderer(RenderConfig& config, ScreenRenderer* scrRenderer)
{
    scrRenderer_ = scrRenderer;
    renderDeferred_ = config.DeferredRenderingEnabled();

    SetupForwardBuffers(config);
    SetupDeferredBuffers(config);
    
    boost::filesystem::path screen_vertex_shader_path = boost::filesystem::path(config.ScreenVertShaderPath()).make_preferred();
    
    if (config.RenderSamples() > 1)
    {
        Framebuffer intermediateBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight());
        intermediateBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        intermediateBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        intermediateBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        mainBuffers_.push_back(intermediateBuffer);
    }
    
    if (config.BloomEnabled())
    {
        bloom_ = true;
        Framebuffer pingBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight());
        pingBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        pingBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        
        Framebuffer pongBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight());
        pongBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        pongBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        
        Framebuffer compositeBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight());
        compositeBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        compositeBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        
        supportBuffers_.push_back(pingBuffer);
        supportBuffers_.push_back(pongBuffer);
        supportBuffers_.push_back(compositeBuffer);
        
        boost::filesystem::path blur_fragment_shader_path = boost::filesystem::path(config.BlurFragShaderPath()).make_preferred();
        blurShader_ = Shader(screen_vertex_shader_path.string().c_str(), blur_fragment_shader_path.string().c_str());
        
        boost::filesystem::path combine_fragment_shader_path = boost::filesystem::path(config.CombineFragShaderPath()).make_preferred();
        combineShader_ = Shader(screen_vertex_shader_path.string().c_str(), combine_fragment_shader_path.string().c_str());
    }
}

void Renderer::SetupForwardBuffers(RenderConfig& config)
{
    // Forward Render Setup
    Framebuffer renderBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight(), config.RenderSamples());
    renderBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
    renderBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
    renderBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    mainBuffers_.push_back(renderBuffer);
}

void Renderer::SetupDeferredBuffers(RenderConfig& config)
{
    // Deferred Render Setup
    Framebuffer compositeBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight(), config.RenderSamples());
    compositeBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
    compositeBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
    compositeBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    mainBuffers_Deferred_.push_back(compositeBuffer);
    
    Framebuffer gBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight(), config.RenderSamples());
    for (int i = 0; i < SceneRenderer::DeferredFramebuffersNumber(); i++)
    {
        gBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
    }
    gBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    mainBuffers_Deferred_.push_back(gBuffer);
}

void Renderer::SwitchRenderMethod()
{
    renderDeferred_ = !renderDeferred_;
}

void Renderer::RenderScene(SceneRenderer& sceneRenderer)
{
    Framebuffer* resultBuffer;
    
    glEnable(GL_DEPTH_TEST);
    if (renderDeferred_)
    {
        resultBuffer = &mainBuffers_Deferred_.front();
        Framebuffer* gBuffer = &mainBuffers_Deferred_[1];
        sceneRenderer.Render_Deferred(*resultBuffer, *gBuffer);
//        resultBuffer = gBuffer;
    }
    else
    {
        resultBuffer = &mainBuffers_.front();
        sceneRenderer.Render_Forward(*resultBuffer);
    }
    glDisable(GL_DEPTH_TEST);
 
    Framebuffer* workingBuffer;
    if (resultBuffer->IsMultiSample())
    {
        Framebuffer* intermediateBuffer = &mainBuffers_.back();
        resultBuffer->DownsampleToFramebuffer(*intermediateBuffer);
        workingBuffer = intermediateBuffer;
    }
    else
    {
        workingBuffer = resultBuffer;
    }
    
    GLuint finalFrame = workingBuffer->RetrieveColorBuffer(0).TargetName;
    if (bloom_)
    {
        GLuint blurredBloom = BlurTexture(workingBuffer->RetrieveColorBuffer(1).TargetName);
        std::vector<GLuint> textures = {finalFrame, blurredBloom};
        finalFrame = CombineTextures(textures);
    }
    
    scrRenderer_->DrawFinalScreenQuad(finalFrame);
}

GLuint Renderer::BlurTexture(GLuint textureName)
{
    supportBuffers_[PINGBUFFER].Use();
    glClear(GL_COLOR_BUFFER_BIT);
    supportBuffers_[PONGBUFFER].Use();
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    int amount = 6;
    GLuint readColorBuffer = textureName;
    unsigned int bufferIndex = 0;
    bool horizontal = true;
    blurShader_.Use();
    for (int i = 0; i < amount; i++)
    {
        supportBuffers_[PINGBUFFER + bufferIndex].Use();
        blurShader_.SetBool("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, readColorBuffer);
        scrRenderer_->DrawPostProcessScreenQuad();
        horizontal = !horizontal;
        readColorBuffer = supportBuffers_[PINGBUFFER + bufferIndex].RetrieveColorBuffer(0).TargetName;
        bufferIndex = (bufferIndex + 1) % 2;
    }
    
    GLuint finalBlur = supportBuffers_[(PINGBUFFER + bufferIndex + 1) % 2].RetrieveColorBuffer(0).TargetName;
    return finalBlur;
}

GLuint Renderer::CombineTextures(std::vector<GLuint> textures)
{
    combineShader_.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    combineShader_.SetInt("texture0", 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    combineShader_.SetInt("texture1", 1);
    
    supportBuffers_[COMPBUFFER].Use();
    scrRenderer_->DrawPostProcessScreenQuad();
    return supportBuffers_[COMPBUFFER].RetrieveColorBuffer(0).TargetName;
}
