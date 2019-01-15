//
//  Input.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/24/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/Input.h"

Input::Input()
{
    cameraInput_ = CameraInput();
    textureMix_ = 0.5f;
    blinnLighting_ = false;
    changeRenderMethod_ = false;
    changePolyFill_ = false;
}

CameraInput Input::CameraInput() const
{
    return cameraInput_;
}

bool Input::BlinnLighting() const
{
    return blinnLighting_;
}

bool Input::ChangeRenderMethod() const
{
    return changeRenderMethod_;
}

bool Input::ChangePolygonFill() const
{
    return changePolyFill_;
}
