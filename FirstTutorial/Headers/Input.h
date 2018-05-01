//
//  Input.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/24/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef Input_h
#define Input_h

class InputWrapper;

struct CameraInput
{
    bool MoveForward : 1;
    bool MoveBack : 1;
    bool MoveLeft : 1;
    bool MoveRight : 1;
    bool RotateRight : 1;
    bool RotateLeft : 1;
    char : 2;
    float x_delta;
    float y_delta;
    float y_offset;
};

class Input
{
friend class InputWrapper;
    
public:
    Input();
    
    CameraInput CameraInput() const;
    bool BlinnLighting() const;
    bool ChangeRenderMethod() const;
    
private:
    struct CameraInput cameraInput_;
    float textureMix_;
    bool blinnLighting_ : 1;
    bool changeRenderMethod_ : 1;
};

#endif /* Input_h */
