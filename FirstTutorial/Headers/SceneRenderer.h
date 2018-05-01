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
#include <glm/glm.hpp>

#include "Headers/Model.h"
#include "Headers/ModelLoader.h"
#include "Headers/Scene.h"
#include "Headers/Shader.h"

class SceneRenderer {
    
    struct ShadowMaps
    {
        std::vector<uint32_t> dirLightShadowMaps;
        std::vector<glm::mat4> dirLightSpaceMats;
        std::vector<uint32_t> pntLightShadowMaps;
        //        std::vector<uint32_t> spotLightShadowMaps_;
        //        std::vector<glm::mat4> spotLightSpaceMats_;
    };
    
    const static std::vector<std::string> deferredTextureNames_;
    const static glm::vec3 bloomThreshold_;
    const static glm::vec3 cubeShadMapDirections_[6];
    const static glm::vec3 cubeShadMapUps_[6];
    
    Scene* scene_;
    ShadowMaps shadowMaps_;
    
    Shader regularShader_;
    Shader transparentShader_;
    Shader outlineShader_;
    Shader skyboxShader_;
    
    Shader deferredGBufferCreationShader_;
    Shader deferredGBufferCompositionShader_;
    Shader transparentShader_Deferred_;
    
    Shader lightsShader_;
    
    Shader dirShadMapMakerShader_;
    Shader pntShadMapMakerShader_;
    Shader sptShadMapMakerShader_;
    
    ModelLoader modelLoader_;
    
public:
    SceneRenderer(Scene* scene, uint32_t shadowRes);
    static uint32_t DeferredFramebuffersNumber();
    
    void Render_Forward(Framebuffer& mainBuffer);
    void Render_Deferred(Framebuffer& compositeBuffer, Framebuffer& gBuffer);
    
private:
    // Should only be called during construction
    void MakeShaders();
    void MakeShadowMaps(uint32_t shadowRes);
    
    void GenerateDirLightShadowMaps(const std::vector<Light>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer);
    void GeneratePntLightShadowMaps(const std::vector<PointLight>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer);
    
    bool sentShadowMaps_;
    void SendShadowMapsToShader(Shader& shader);
    
    void DrawLights(SceneLighting& lighting, Shader& lightsShader);
    
    void CompositeDeferredRenderTextures(Framebuffer& gBuffer, Shader& compositionShader);
    void RenderDrawLists(std::vector<std::vector<Object>> drawLists, Shader& shader);
    void RenderObjectsInstanced(std::vector<Object> drawList, Shader& shader);
};

#endif /* SCENERENDERER_H */
