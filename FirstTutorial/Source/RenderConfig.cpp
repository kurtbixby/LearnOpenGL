//
//  RenderConfig.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/12/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/RenderConfig.h"

#include <cstdint>

RenderConfig::RenderConfig()
{
    windowWidth_ = 800;
    windowHeight_ = 600;
    samples_ = 4;
    bloom_ = true;
    shadowmaps_ = true;
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
    return windowWidth_;
}

uint32_t RenderConfig::WindowHeight()
{
    return windowHeight_;
}

uint32_t RenderConfig::RenderWidth()
{
    return windowWidth_;
}

uint32_t RenderConfig::RenderHeight()
{
    return windowHeight_;
}

uint32_t RenderConfig::RenderSamples()
{
    return samples_;
}

bool RenderConfig::BloomEnabled()
{
    return bloom_;
}

bool RenderConfig::ShadowmapsEnabled()
{
    return shadowmaps_;
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
