//
//  Renderer.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/19/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <glad/glad.h>
#include <vector>

#include "Headers/Framebuffer.h"
#include "Headers/RenderConfig.h"
#include "Headers/SceneRenderer.h"
#include "Headers/ScreenRenderer.h"
#include "Headers/Shader.h"

class Renderer {
    ScreenRenderer* scrRenderer_;
    
    Shader blurShader_;
    Shader combineShader_;
    
    bool renderDeferred_;
    bool bloom_;
    
    std::vector<Framebuffer> mainBuffers_;
    std::vector<Framebuffer> supportBuffers_;
    
    std::vector<Framebuffer> mainBuffers_Deferred_;
public:
    Renderer(RenderConfig& config, ScreenRenderer* scrRenderer);
    void RenderScene(SceneRenderer& sceneRenderer);
    void SwitchRenderMethod();
    
private:
    // Initialization
    void SetupForwardBuffers(RenderConfig& config);
    void SetupDeferredBuffers(RenderConfig& config);
    
    // Utility
    GLuint BlurTexture(GLuint textureName);
    GLuint CombineTextures(std::vector<GLuint> textures);
};

#endif /* RENDERER_H */
