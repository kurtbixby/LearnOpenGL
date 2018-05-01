//
//  ScreenRenderer.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 5/1/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef SCREENRENDERER_H
#define SCREENRENDERER_H

#include <stdio.h>
#include <glad/glad.h>

#include "Headers/RenderConfig.h"
#include "Headers/Shader.h"

class ScreenRenderer {
    GLuint screenQuadVAO_;
    Shader screenQuadShader_;
    
public:
    ScreenRenderer(RenderConfig& config);
    void DrawFinalScreenQuad(GLuint finalFrameTexture);
    void DrawPostProcessScreenQuad();
    
private:
    void CreateScreenQuadVAO();
    void CreateScreenShader(RenderConfig& config);
};

#endif /* SCREENRENDERER_H */
