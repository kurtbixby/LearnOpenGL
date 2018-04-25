//
//  SceneRenderer.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>

#include "Headers/Model.h"
#include "Headers/ModelLoader.h"
#include "Headers/Scene.h"
#include "Headers/Shader.h"

class SceneRenderer {
    Shader regularShader_;
    Shader transparentShader_;
    Shader outlineShader_;
    Shader skyboxShader_;
    
    Shader deferredTextureCreationShader_;
    Shader deferredTextureCompositionShader_;
    Shader transparentShader_Deferred_;
    
    Shader makeDirShadMapShader_;
    Shader makePntShadMapShader_;
    Shader makeSptShadMapShader_;
    
    ModelLoader modelLoader_;
    
    const static std::vector<std::string> deferredTextureNames_;
    
public:
    SceneRenderer();
    
    std::vector<std::vector<GLuint>> MakeShadowMaps(Scene& scene);
    
    // Create shadow maps
    GLuint RenderScene_Forward(Scene& scene);
    GLuint RenderScene_Deferred(Scene& scene, uint32_t width_, uint32_t height_);
    
private:
    void CompositeDeferredRenderTextures(Framebuffer& deferredBuffer, Shader& compositionShader);
    void RenderDrawLists(std::vector<std::vector<Object>> drawLists, Shader& shader);
    void RenderObjectsInstanced(std::vector<Object> drawList, Shader& shader);
};

#endif /* SCENERENDERER_H */
