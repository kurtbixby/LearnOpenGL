//
//  Debug.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/23/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef Debug_h
#define Debug_h

#include <glad/glad.h>
#include <iostream>

namespace debug
{
    void openGL_Errors()
    {
        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            switch (err)
            {
                case GL_INVALID_ENUM:
                    std::cout << "GL INVALID ENUM" << std::endl;
                    continue;
                case GL_INVALID_VALUE:
                    std::cout << "GL_INVALID_VALUE" << std::endl;
                    continue;
                case GL_INVALID_OPERATION:
                    std::cout << "GL_INVALID_OPERATION" << std::endl;
                    continue;
                    //            case GL_STACK_OVERFLOW:
                    //                break;
                    //            case GL_STACK_UNDERFLOW:
                    //                break;
                case GL_OUT_OF_MEMORY:
                    std::cout << "GL_OUT_OF_MEMORY" << std::endl;
                    continue;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
                    continue;
                case GL_CONTEXT_LOST:
                    std::cout << "GL_CONTEXT_LOST" << std::endl;
                    continue;
                default:
                    break;
            }
        }
    }
}

#endif /* Debug_h */
