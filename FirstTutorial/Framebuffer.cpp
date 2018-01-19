#include "Framebuffer.h"

#include <glad/glad.h>
#include <vector>
#include <algorithm>

#include "Structs.h"

// Texture size has to change on Mac. Cause of skybox issue?
Framebuffer::Framebuffer(): Framebuffer(800, 600, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, GLenum target)
{
    width_ = width;
    height_ = height;
    target_ = target;

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    fbo_ = fbo;

    colorAttachments_ = std::vector<RenderTarget>();
    colorAttachmentCount_ = 0;

    depthAttachment_ = RenderTarget {0, RenderTargetType::None};
    stencilAttachment_ = RenderTarget {0, RenderTargetType::None};
    depthStencilAttachment_ = RenderTarget {0, RenderTargetType::None};
}

void Framebuffer::Use()
{
    Use(target_);
}

void Framebuffer::Use(GLenum target)
{
    target_ = target;
    glBindFramebuffer(target_, fbo_);
}

void Framebuffer::UseDefault(GLenum target)
{
	glBindFramebuffer(target, 0);
}

bool Framebuffer::IsComplete()
{
    glBindFramebuffer(target_, fbo_);
    bool isComplete = glCheckFramebufferStatus(target_) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(target_, 0);

    return isComplete;
}

RenderTarget Framebuffer::RetrieveColorBuffer(unsigned int bufferNumber)
{
    if (colorAttachmentCount_ == 0)
    {
        return RenderTarget {0, RenderTargetType::None};
    }
    unsigned int index = std::min(bufferNumber, colorAttachmentCount_ - 1);
    return colorAttachments_[index];
}

RenderTarget Framebuffer::RetrieveDepthBuffer()
{
    return RenderTarget {0, RenderTargetType::None};
}

RenderTarget Framebuffer::RetrieveStencilBuffer()
{
    return RenderTarget {0, RenderTargetType::None};
}

RenderTarget Framebuffer::RetrieveDepthStencilBuffer()
{
    return RenderTarget {0, RenderTargetType::None};
}

void Framebuffer::AddTextureAttachment(FBAttachment attachmentType)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int fbTexture;
    switch (attachmentType)
    {
        case FBAttachment::Color:
            fbTexture = CreateFramebufferTexture(attachmentType);
            break;
        case FBAttachment::DepthStencil:
            fbTexture = CreateFramebufferTexture(attachmentType);
            break;
    }

    RenderTarget renderTarget = RenderTarget {fbTexture, RenderTargetType::Texture};

    switch (attachmentType)
    {
        case FBAttachment::Color:
            glFramebufferTexture2D(target_, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, GL_TEXTURE_2D, fbTexture, 0);
            colorAttachments_.push_back(renderTarget);
            colorAttachmentCount_ += 1;
            break;
        case FBAttachment::DepthStencil:
            glFramebufferTexture2D(target_, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbTexture, 0);
            depthStencilAttachment_ = renderTarget;
            break;
    }

    glBindFramebuffer(target_, 0);
}

void Framebuffer::AddRenderbufferAttachment(FBAttachment attachmentType)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int renderbuffer;
    switch (attachmentType)
    {
        case FBAttachment::Color:
            renderbuffer = CreateRenderBuffer(attachmentType);
            break;
        case FBAttachment::DepthStencil:
            renderbuffer = CreateRenderBuffer(attachmentType);
            break;
    }

    RenderTarget renderTarget = RenderTarget {renderbuffer, RenderTargetType::Renderbuffer};

    switch (attachmentType)
    {
        case FBAttachment::Color:
            glFramebufferRenderbuffer(target_, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, GL_RENDERBUFFER, renderbuffer);
            colorAttachments_.push_back(renderTarget);
            colorAttachmentCount_ += 1;            
            break;
        case FBAttachment::DepthStencil:
            glFramebufferRenderbuffer(target_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
            depthStencilAttachment_ = renderTarget;
            break;
    }

    glBindFramebuffer(target_, 0);
}

// While this saves 20 lines, it's much more confusing
// void Framebuffer::AddAttachment(RenderTargetType targetType, FBAttachment attachmentType)
// {
//     glBindFramebuffer(target_, fbo_);

//     unsigned int renderObject;

//     switch (targetType)
//     {
//         case RenderTargetType::Texture:
//             renderObject = CreateFramebufferTexture(attachmentType);
//             break;
//         case RenderTargetType::Renderbuffer:
//             renderObject = CreateRenderBuffer(attachmentType);
//             break;
//     }

//     RenderTarget renderTarget = RenderTarget {renderObject, targetType};

//     switch (attachmentType)
//     {
//         case FBAttachment::Color:
//             switch (targetType)
//             {
//                 case RenderTargetType::Texture:
//                     glFramebufferTexture2D(target_, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, GL_TEXTURE_2D, fbTexture, 0);
//                     break;
//                 case RenderTargetType::Renderbuffer:
//                     glFramebufferRenderbuffer(target_, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, GL_RENDERBUFFER, renderbuffer);
//                     break;
//             }
//             colorAttachments_.push_back(renderTarget);
//             colorAttachmentCount_ += 1;
//             break;
//         case FBAttachment::DepthStencil:
//             switch (targetType)
//             {
//                 case RenderTargetType::Texture:
//                     glFramebufferTexture2D(target_, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbTexture, 0);
//                     break;
//                 case RenderTargetType::Renderbuffer:
//                     glFramebufferRenderbuffer(target_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
//                     break;
//             }
//             depthStencilAttachment_ = renderTarget;            
//             break;
//     }
// }

// Framebuffer Texture Generator
unsigned int Framebuffer::CreateFramebufferTexture(FBAttachment attachmentType)
{
    unsigned int fbTex;

    switch(attachmentType)
    {
        case FBAttachment::Color:
            fbTex = GenFramebufferTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
            break;
        case FBAttachment::DepthStencil:
            fbTex = GenFramebufferTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
            break;
    }

    return fbTex;
}

unsigned int Framebuffer::CreateRenderBuffer(FBAttachment attachmentType)
{
    unsigned int renderbuffer;

    switch(attachmentType)
    {
        case FBAttachment::Color:
            renderbuffer = GenRenderbuffer(GL_RGB);
            break;
        case FBAttachment::DepthStencil:
            renderbuffer = GenRenderbuffer(GL_DEPTH24_STENCIL8);
            break;
    }

    return renderbuffer;
}

unsigned int Framebuffer::GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0, format, dataType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

unsigned int Framebuffer::GenRenderbuffer(GLenum internalFormat)
{
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width_, height_);
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rbo;
}

// void texturebuffer()
// {
//     unsigned int fbo = createFramebuffer();

//     // Other buffers are GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER
//     glBindFramebuffer(GL_FRAMEBUFFER, fbo);

//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//     {
//         // Fail
//     }
//     else
//     {
//         // Draw to new framebuffer
//     }

//     // Rebind default
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }
