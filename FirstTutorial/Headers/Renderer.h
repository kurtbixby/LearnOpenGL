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
#include "Headers/Shader.h"

class Renderer {
    GLuint screenQuadVAO_;
    std::vector<Framebuffer> buffers_;
    Shader screenQuadShader_;
    
    std::vector<Framebuffer> supportBuffers_;
    Shader blurShader_;
    Shader combineShader_;
    
    bool bloom_;
public:
    Renderer(RenderConfig& config);
    void RenderScene(SceneRenderer& sceneRenderer);
    
private:
    // Initialization
    GLuint CreateScreenQuadVAO();
    
    // Drawing
    void DrawScreenQuad();
    
    // Utility
    GLuint BlurTexture(GLuint textureName);
    GLuint CombineTextures(std::vector<GLuint> textures);
};

#endif /* RENDERER_H */
