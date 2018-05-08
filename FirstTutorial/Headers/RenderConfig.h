//
//  RenderConfig.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/12/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

// FIND CONFIG LIBRARY TO REPLACE MOST OF THIS CLASS WITH

#ifndef RENDERCONFIG_H
#define RENDERCONFIG_H

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <string>

class RenderConfig {
    uint32_t samples_;
    uint32_t renderWidth_;
    uint32_t renderHeight_;
    uint32_t shadowMapRes_;
    float gamma_;
    float exposure_;
    bool shadowmaps_;
    bool bloom_;
    bool deferredRendering_;
    bool ssao_;
    std::vector<float> kernel_;
    std::string screenVertShaderPath_;
    std::string screenFragShaderPath_;
    std::string blurFragShaderPath_;
    std::string combineFragShaderPath_;
    
public:
    RenderConfig();
    RenderConfig(uint32_t renderWidth, uint32_t renderHeight);
    
    uint32_t WindowWidth();
    uint32_t WindowHeight();
    uint32_t RenderWidth();
    uint32_t RenderHeight();
    uint32_t RenderSamples();
    uint32_t ShadowMapRes();
    
    bool BloomEnabled();
    bool ShadowmapsEnabled();
    bool DeferredRenderingEnabled();
    bool SSAOEnabled();
    
    float ScreenGamma();
    float ScreenExposure();
    
    std::vector<float> ScreenFXKernel();
    
    std::string ScreenVertShaderPath();
    std::string ScreenFragShaderPath();
    std::string BlurFragShaderPath();
    std::string CombineFragShaderPath();
};

#endif /* RENDERCONFIG_H */
