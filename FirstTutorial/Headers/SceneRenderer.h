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
#include "Headers/RenderConfig.h"
#include "Headers/Scene.h"
#include "Headers/ScreenRenderer.h"
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
    const static std::vector<std::string> deferredSSAOTextureNames_;
    const static glm::vec3 bloomThreshold_;
    const static glm::vec3 cubeShadMapDirections_[6];
    const static glm::vec3 cubeShadMapUps_[6];
    
    RenderConfig* renderConfig_;
    
    ScreenRenderer* scrRenderer_;
    
    Scene* scene_;
    
    bool ssaoEnabled_;
    Framebuffer ssaoBuffer_;
    Framebuffer ssaoBlurBuffer_;
    
    Framebuffer forwardBuffer_;
    
    Framebuffer gBuffer_;
    Framebuffer deferredCompBuffer_;
    
    ShadowMaps shadowMaps_;
    bool sentShadowMaps_;
    
    Shader regularShader_;
    Shader transparentShader_;
    Shader outlineShader_;
    Shader skyboxShader_;
    
    Shader deferredGBufferCreationShader_;
    Shader deferredGBufferCompositionShader_;
    Shader transparentShader_Deferred_;
    Shader deferredSkyboxShader_;
    Shader deferredSSAOShader_;
    Shader ssaoBlurShader_;
    
    Shader lightsShader_;
    
    Shader dirShadMapMakerShader_;
    Shader pntShadMapMakerShader_;
    Shader sptShadMapMakerShader_;
    
    ModelLoader modelLoader_;
    
public:
    SceneRenderer(Scene* scene, ScreenRenderer* scrRenderer, RenderConfig* renderConfig);
    static uint32_t DeferredFramebuffersNumber();
    
    void RenderChanged();
    
    const Framebuffer* Render_Forward();
    const Framebuffer* Render_Deferred();
    
private:
    
    /*
     *  Initialization Methods
     */
    void MakeFramebuffers(uint32_t renderWidth, uint32_t renderHeight, uint32_t samples = 1); // Doesn't make multisampled gBuffer
    void MakeShaders();
    void MakeShadowMaps(uint32_t shadowRes);
    void SSAO_Setup();
    
    void GenerateDirLightShadowMaps(const std::vector<Light>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer);
    void GeneratePntLightShadowMaps(const std::vector<PointLight>& lights, const std::vector<std::vector<Object>> &regularDrawLists, Framebuffer& shadowFramebuffer);
    
    void SendShadowMapsToShader(Shader& shader);
    
    void DrawLights(SceneLighting& lighting, Shader& lightsShader);
    
    void CompositeDeferredRenderTextures(Shader& compositionShader, Framebuffer& gBuffer, Framebuffer& ssaoBuffer);
    void RenderDrawLists(std::vector<std::vector<Object>> drawLists, Shader& shader);
    void RenderObjectsInstanced(std::vector<Object> drawList, Shader& shader, GLuint fillMode = GL_FILL);
    
    // SSAO Methods: MOVE THESE
    std::vector<glm::vec3> SSAO_SampleOffsets(uint32_t sampleNumber);
    float lerp(float a, float b, float t);
    GLuint SSAO_NoiseTexture();
    void SSAO_SendConstantData(Shader& shader, std::vector<glm::vec3>& sampleOffsets, GLuint noiseTex);
    void SSAO_SendViewTextures(Shader& shader, Framebuffer& gBuffer);
};

#endif /* SCENERENDERER_H */
