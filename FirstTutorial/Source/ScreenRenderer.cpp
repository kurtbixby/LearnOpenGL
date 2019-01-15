//
//  ScreenRenderer.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 5/1/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/ScreenRenderer.h"

#include <glad/glad.h>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include "Headers/Framebuffer.h"
#include "Headers/RenderConfig.h"
#include "Headers/Shader.h"

ScreenRenderer::ScreenRenderer(RenderConfig& config)
{
    CreateScreenQuadVAO();
    CreateScreenShader(config);
}

void ScreenRenderer::DrawPostProcessScreenQuad()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(screenQuadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void ScreenRenderer::DrawFinalScreenQuad(GLuint finalFrameTexture)
{
    Framebuffer::UseDefault();
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    screenQuadShader_.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, finalFrameTexture);
    DrawPostProcessScreenQuad();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ScreenRenderer::CreateScreenQuadVAO()
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
    
    screenQuadVAO_ = quadVAO;
}

void ScreenRenderer::CreateScreenShader(RenderConfig& config)
{
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
}
