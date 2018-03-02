#include "Headers/Framebuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>
#include <algorithm>

//#include "Headers/Debug.h"
#include "Headers/Structs.h"


#define BASEFRAMEBUFFERH 800
#define BASEFRAMEBUFFERV 600

#ifdef __APPLE__
#define FRAMEBUFFERH 2 * BASEFRAMEBUFFERH
#define FRAMEBUFFERV 2 * BASEFRAMEBUFFERV
#else
#define FRAMEBUFFERH BASEFRAMEBUFFERH
#define FRAMEBUFFERV BASEFRAMEBUFFERV
#endif

// Texture size has to change on Mac. Cause of skybox issue?
Framebuffer::Framebuffer(): Framebuffer(FRAMEBUFFERH, FRAMEBUFFERV, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, bool useColorBuffer, GLenum target)
{
    width_ = width;
    height_ = height;
    target_ = target;
    noColor_ = !useColorBuffer;

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    fbo_ = fbo;

    glBindFramebuffer(target_, fbo_);
    if (noColor_)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    glBindFramebuffer(target_, 0);
    
    colorAttachments_ = std::vector<RenderTarget>();
    colorAttachmentCount_ = 0;

    depthAttachment_ = RenderTarget {0, RenderTargetType::None};
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

void Framebuffer::SetViewPort()
{
    glViewport(0, 0, width_, height_);
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
    
    RenderTarget attachment = colorAttachments_[index];
    
    return attachment;
}

void Framebuffer::DownsampleToFramebuffer(Framebuffer& other_fb)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other_fb.fbo_);
    glBlitFramebuffer(0, 0, this->width_, this->height_, 0, 0, other_fb.width_, other_fb.height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

RenderTarget Framebuffer::RetrieveDepthBuffer()
{
    return depthAttachment_;
}

RenderTarget Framebuffer::RetrieveDepthStencilBuffer()
{
    return depthStencilAttachment_;
}

void Framebuffer::AddTextureAttachment(FBAttachment attachmentType, uint32_t samples)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int fbTexture = CreateFramebufferTexture(attachmentType, samples);
//    switch (attachmentType)
//    {
//        case FBAttachment::Color:
//            fbTexture = CreateFramebufferTexture(attachmentType, samples);
//            break;
//        case FBAttachment::DepthStencil:
//            fbTexture = CreateFramebufferTexture(attachmentType, samples);
//            break;
//        case FBAttachment::Depth:
//            break;
//        case FBAttachment::Stencil:
//            break;
//    }

    RenderTarget renderTarget = RenderTarget {fbTexture, RenderTargetType::Texture};

    auto textureSampleType = GL_TEXTURE_2D;
    if (samples != 1)
    {
        textureSampleType = GL_TEXTURE_2D_MULTISAMPLE;
    }
    
    switch (attachmentType)
    {
        case FBAttachment::Color:
            glFramebufferTexture2D(target_, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, textureSampleType, fbTexture, 0);
            colorAttachments_.push_back(renderTarget);
            colorAttachmentCount_ += 1;
            break;
        case FBAttachment::DepthStencil:
            glFramebufferTexture2D(target_, GL_DEPTH_STENCIL_ATTACHMENT, textureSampleType, fbTexture, 0);
            depthStencilAttachment_ = renderTarget;
            break;
        case FBAttachment::Depth:
            glFramebufferTexture2D(target_, GL_DEPTH_ATTACHMENT, textureSampleType, fbTexture, 0);
            depthAttachment_ = renderTarget;
            break;
    }

    glBindFramebuffer(target_, 0);
}

void Framebuffer::AddRenderbufferAttachment(FBAttachment attachmentType, uint32_t samples)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int renderbuffer = CreateRenderBuffer(attachmentType, samples);
//    switch (attachmentType)
//    {
//        case FBAttachment::Color:
//            renderbuffer = CreateRenderBuffer(attachmentType, samples);
//            break;
//        case FBAttachment::DepthStencil:
//            renderbuffer = CreateRenderBuffer(attachmentType, samples);
//            break;
//    }

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
        case FBAttachment::Depth:
            glFramebufferRenderbuffer(target_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
            depthAttachment_ = renderTarget;
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
unsigned int Framebuffer::CreateFramebufferTexture(FBAttachment attachmentType, uint32_t samples)
{
    unsigned int fbTex;

    switch(attachmentType)
    {
        case FBAttachment::Color:
            fbTex = GenFramebufferTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, samples);
            break;
        case FBAttachment::DepthStencil:
            fbTex = GenFramebufferTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, samples);
            break;
        case FBAttachment::Depth:
            fbTex = GenFramebufferTexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, samples);
            break;
    }

    return fbTex;
}

unsigned int Framebuffer::CreateRenderBuffer(FBAttachment attachmentType, uint32_t samples)
{
    unsigned int renderbuffer;

    switch(attachmentType)
    {
        case FBAttachment::Color:
            renderbuffer = GenRenderbuffer(GL_RGB, samples);
            break;
        case FBAttachment::DepthStencil:
            renderbuffer = GenRenderbuffer(GL_DEPTH24_STENCIL8, samples);
            break;
        case FBAttachment::Depth:
            renderbuffer = GenRenderbuffer(GL_DEPTH_COMPONENT, samples);
            break;
    }

    return renderbuffer;
}

unsigned int Framebuffer::GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType, uint32_t samples)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    
    bool multisampled = samples > 1;
    auto sampleType = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    
    glBindTexture(sampleType, texture);
    
    if (multisampled)
    {
        glTexImage2DMultisample(sampleType, samples, internalFormat, width_, height_, GL_TRUE);
    }
    else
    {
        glTexImage2D(sampleType, 0, internalFormat, width_, height_, 0, format, dataType, nullptr);
        glTexParameteri(sampleType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(sampleType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL)
    {
        glTexParameteri(sampleType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(sampleType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(sampleType, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));
    }
    
    glBindTexture(sampleType, 0);
    return texture;
}

unsigned int Framebuffer::GenRenderbuffer(GLenum internalFormat, uint32_t samples)
{
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    if (samples == 1)
    {
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width_, height_);
    }
    else
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width_, height_);
    }
    
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
