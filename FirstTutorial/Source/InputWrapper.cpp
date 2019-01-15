//
//  InputWrapper.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/24/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/InputWrapper.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

#include <Headers/CompilerWarning.h>

InputWrapper::InputWrapper()
{
    input_ = Input();
    
    textureMix_ = 0.5f;
    mixIncrement_ = 0.01f;
    mixMin_ = 0.0f;
    mixMax_ = 1.0f;
    
    firstMouse_ = true;
    currentX_ = 0.0f;
    currentY_ = 0.0f;
    lastX_ = 0.0f;
    lastY_ = 0.0f;
    
    yOffset_ = 0.0f;
}

void InputWrapper::MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    currentX_ = xPos;
    currentY_ = yPos;
    // If uninitialized
    if (firstMouse_)
    {
        lastX_ = currentX_;
        lastY_ = currentY_;
        firstMouse_ = false;
    }
}

void InputWrapper::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    yOffset_ += yOffset;
}

void InputWrapper::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
#pragma message WARN("Implement this. Remember to reset the state every frame/tick.")
}

Input InputWrapper::TakeInput(GLFWwindow* window)
{
    CameraInput cameraInput = CameraInput();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        input_.textureMix_ = std::min(input_.textureMix_ + mixIncrement_, mixMax_);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        input_.textureMix_ = std::max(input_.textureMix_ - mixIncrement_, mixMin_);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraInput.MoveForward = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraInput.MoveBack = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraInput.MoveRight = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraInput.MoveLeft = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraInput.RotateRight = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraInput.RotateLeft = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        input_.blinnLighting_ ^= 1;
    }
    
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        input_.changeRenderMethod_ = 1;
    }
    else
    {
        input_.changeRenderMethod_ = 0;
    }
    
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        input_.changePolyFill_ = 1;
    }
    else
    {
        input_.changePolyFill_ = 0;
    }
    
    float x_delta = currentX_ - lastX_;
    float y_delta = lastY_ - currentY_;
    
    lastX_ = currentX_;
    lastY_ = currentY_;
    
    cameraInput.x_delta = x_delta;
    cameraInput.y_delta = y_delta;
    
    cameraInput.y_offset = yOffset_;
    yOffset_ = 0.0f;
    
    input_.cameraInput_ = cameraInput;
    
    return input_;
}
