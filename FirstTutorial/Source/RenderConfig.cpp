//
//  RenderConfig.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/12/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/RenderConfig.h"

#include <cstdint>

// Texture size has to change on Mac. Cause of skybox issue?
#ifdef __APPLE__
#define FRAMEBUFFERMULTIPLIER 2
#else
#define FRAMEBUFFERMULTIPLIER 1
#endif

RenderConfig::RenderConfig() : RenderConfig(1280, 720)
{ }

RenderConfig::RenderConfig(uint32_t renderWidth, uint32_t renderHeight)
{
    renderWidth_ = renderWidth;
    renderHeight_ = renderHeight;
    shadowMapRes_ = 1024;
    samples_ = 1;
    bloom_ = true;
    shadowmaps_ = true;
    deferredRendering_ = true;
    ssao_ = true;
    gamma_ = 2.2f;
    exposure_ = 1.0f;
    kernel_ = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    };
    
    screenVertShaderPath_ = "Shaders/Screen.vert";
    screenFragShaderPath_ = "Shaders/Kernel_Gamma_HDR_Exposure.frag";
    blurFragShaderPath_ = "Shaders/Blur.frag";
    combineFragShaderPath_ = "Shaders/Combine.frag";
}

uint32_t RenderConfig::WindowWidth()
{
    return renderWidth_ / FRAMEBUFFERMULTIPLIER;
}

uint32_t RenderConfig::WindowHeight()
{
    return renderHeight_ / FRAMEBUFFERMULTIPLIER;
}

uint32_t RenderConfig::RenderWidth()
{
    return renderWidth_;
}

uint32_t RenderConfig::RenderHeight()
{
    return renderHeight_;
}

uint32_t RenderConfig::RenderSamples()
{
    return samples_;
}

uint32_t RenderConfig::ShadowMapRes()
{
    return shadowMapRes_;
}

bool RenderConfig::BloomEnabled()
{
    return bloom_;
}

bool RenderConfig::ShadowmapsEnabled()
{
    return shadowmaps_;
}

bool RenderConfig::DeferredRenderingEnabled()
{
    return deferredRendering_;
}

bool RenderConfig::SSAOEnabled()
{
    return ssao_;
}

float RenderConfig::ScreenGamma()
{
    return gamma_;
}

float RenderConfig::ScreenExposure()
{
    return exposure_;
}

std::vector<float> RenderConfig::ScreenFXKernel()
{
    return kernel_;
}

std::string RenderConfig::ScreenVertShaderPath()
{
    return screenVertShaderPath_;
}

std::string RenderConfig::ScreenFragShaderPath()
{
    return screenFragShaderPath_;
}

std::string RenderConfig::BlurFragShaderPath()
{
    return blurFragShaderPath_;
}

std::string RenderConfig::CombineFragShaderPath()
{
    return combineFragShaderPath_;
}
