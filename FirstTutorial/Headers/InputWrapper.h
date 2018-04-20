//
//  InputWrapper.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/24/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef InputWrapper_h
#define InputWrapper_h

#include <GLFW/glfw3.h>
#include "Headers/Input.h"

class InputWrapper
{
public:
    InputWrapper();
    
    Input TakeInput(GLFWwindow* window);
    
    void MouseCallback(GLFWwindow* window, double xPos, double yPos);
    void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
private:
    Input input_;
    
    float textureMix_;
    float mixIncrement_;
    float mixMin_;
    float mixMax_;
    
    bool firstMouse_;
    float currentX_;
    float currentY_;
    float lastX_;
    float lastY_;
    
    // Possible to not store this?
    float yOffset_;
};

#endif /* InputWrapper_h */
