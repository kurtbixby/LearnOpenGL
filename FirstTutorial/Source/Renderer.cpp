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
#include "Headers/Shader.h"

#define PINGBUFFER 0
#define PONGBUFFER 1
#define COMPBUFFER 2

Renderer::Renderer(RenderConfig& config)
{
    {
        Framebuffer primaryBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight(), config.RenderSamples());
        primaryBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        primaryBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        primaryBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        buffers_.push_back(primaryBuffer);
    }
    
    boost::filesystem::path screen_vertex_shader_path = boost::filesystem::path(config.ScreenVertShaderPath()).make_preferred();
    boost::filesystem::path screen_fragment_shader_path = boost::filesystem::path(config.ScreenFragShaderPath()).make_preferred();
    screenQuadShader_ = Shader(screen_vertex_shader_path.string().c_str(), screen_fragment_shader_path.string().c_str());
    
    screenQuadShader_.Use();
    screenQuadShader_.SetFloat("gamma", config.ScreenGamma());
    screenQuadShader_.SetFloat("exposure", config.ScreenExposure());
    
    std::vector<float> kernel = config.ScreenFXKernel();
    while (kernel.size() < 9)
    {
        kernel.push_back(0);
    }
    screenQuadShader_.SetFloats("kernel", 9, kernel.data());
    
    screenQuadVAO_ = CreateScreenQuadVAO();
    
    if (config.RenderSamples() > 1)
    {
        Framebuffer intermediateBuffer = Framebuffer(config.RenderWidth(), config.RenderHeight());
        intermediateBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        intermediateBuffer.AddTextureAttachment(FBAttachment::ColorHDR);
        intermediateBuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);
        buffers_.push_back(intermediateBuffer);
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

GLuint Renderer::CreateScreenQuadVAO()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    
    GLuint quadVAO;
    glGenVertexArrays(1, &quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2));
    glBindVertexArray(0);
    
    return quadVAO;
}

void Renderer::RenderScene(SceneRenderer& sceneRenderer)
{
    Framebuffer& primaryBuffer = buffers_.front();
    primaryBuffer.SetViewPort();
    primaryBuffer.Use();

    // Need to specify if bloom is enabled
    glEnable(GL_DEPTH_TEST);
    sceneRenderer.Render_Forward(primaryBuffer);
    glDisable(GL_DEPTH_TEST);
 
    Framebuffer workingBuffer;
    if (primaryBuffer.IsMultiSample())
    {
        Framebuffer& intermediateBuffer = buffers_[1];
        primaryBuffer.DownsampleToFramebuffer(intermediateBuffer);
        workingBuffer = intermediateBuffer;
    }
    else
    {
        workingBuffer = primaryBuffer;
    }
    
    GLuint finalFrame = workingBuffer.RetrieveColorBuffer(0).TargetName;
    if (bloom_)
    {
        GLuint blurredBloom = BlurTexture(workingBuffer.RetrieveColorBuffer(1).TargetName);
        std::vector<GLuint> textures = {finalFrame, blurredBloom};
        finalFrame = CombineTextures(textures);
    }
    
    Framebuffer::UseDefault();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    screenQuadShader_.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, finalFrame);
    DrawScreenQuad();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::DrawScreenQuad()
{
    glBindVertexArray(screenQuadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint Renderer::BlurTexture(GLuint textureName)
{
    supportBuffers_[PINGBUFFER].Use();
    glClear(GL_COLOR_BUFFER_BIT);
    supportBuffers_[PONGBUFFER].Use();
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(screenQuadVAO_);
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
        glDrawArrays(GL_TRIANGLES, 0, 6);
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
    DrawScreenQuad();
    return supportBuffers_[COMPBUFFER].RetrieveColorBuffer(0).TargetName;
}
