#include "Headers/Framebuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>
#include <algorithm>

//#include "Headers/Debug.h"
#include "Headers/Structs.h"

// Texture size has to change on Mac. Cause of skybox issue?
#ifdef __APPLE__
#define FRAMEBUFFERMULTIPLIER 2
#else
#define FRAMEBUFFERMULTIPLIER 1
#endif

Framebuffer::Framebuffer(): Framebuffer(800, 600, 1, true, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(uint32_t width, uint32_t height): Framebuffer(width, height, 1, true, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, uint32_t samples, bool useColorBuffer, GLenum target)
{
    width_ = FRAMEBUFFERMULTIPLIER * width;
    height_ = FRAMEBUFFERMULTIPLIER * height;
    target_ = target;
    noColor_ = !useColorBuffer;
    samples_ = samples;
    hasAttachments_ = false;

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

void Framebuffer::SetSamples(uint32_t samples)
{
    if (!hasAttachments_)
    {
        samples_ = samples;
    }
}

void Framebuffer::Use()
{
    Use(target_);
}

void Framebuffer::Use(GLenum target)
{
    target_ = target;
    glBindFramebuffer(target_, fbo_);
    
    if (colorAttachmentCount_ > 0)
    {
        std::vector<unsigned int> attachments = std::vector<unsigned int>();
        for (int i = 0; i < colorAttachmentCount_; i++)
        {
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glDrawBuffers(colorAttachmentCount_, &attachments[0]);
    }
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

bool Framebuffer::IsMultiSample()
{
    return samples_ > 1;
}

float Framebuffer::AspectRatio()
{
    return float(width_) / float(height_);
}

glm::vec2 Framebuffer::Center()
{
    return glm::vec2(width_ / 2, height_ / 2);
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
    if (this->width_ == other_fb.width_ && this->height_ == other_fb.height_ && this->samples_ == other_fb.samples_)
    {
        // don't do anything
        // ADD BETTER ERROR HANDLING
        return;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other_fb.fbo_);
    for (int i = 0; i < colorAttachmentCount_; i++)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
        glBlitFramebuffer(0, 0, this->width_, this->height_, 0, 0, other_fb.width_, other_fb.height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
}

RenderTarget Framebuffer::RetrieveDepthBuffer()
{
    return depthAttachment_;
}

RenderTarget Framebuffer::RetrieveDepthStencilBuffer()
{
    return depthStencilAttachment_;
}

void Framebuffer::AddTextureAttachment(FBAttachment attachmentType)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int fbTexture = CreateFramebufferTexture(attachmentType);

    RenderTarget renderTarget = RenderTarget {fbTexture, RenderTargetType::Texture};

    auto textureSampleType = GL_TEXTURE_2D;
    if (samples_ != 1)
    {
        textureSampleType = GL_TEXTURE_2D_MULTISAMPLE;
    }
    
    switch (attachmentType)
    {
        case FBAttachment::Color:
        case FBAttachment::ColorHDR:
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

void Framebuffer::AddRenderbufferAttachment(FBAttachment attachmentType)
{
    glBindFramebuffer(target_, fbo_);

    unsigned int renderbuffer = CreateRenderBuffer(attachmentType);

    RenderTarget renderTarget = RenderTarget {renderbuffer, RenderTargetType::Renderbuffer};

    switch (attachmentType)
    {
        case FBAttachment::Color:
        case FBAttachment::ColorHDR:
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

void Framebuffer::AddCubemapAttachment(FBAttachment attachmentType)
{
    glBindFramebuffer(target_, fbo_);
    
    unsigned int cubemap = CreateCubemapTexture(attachmentType);
    
    RenderTarget renderTarget = RenderTarget {cubemap, RenderTargetType::Cubemap};
    
    // Delete/remove old texture
    switch (attachmentType)
    {
        case FBAttachment::Color:
        case FBAttachment::ColorHDR:
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentCount_, cubemap, 0);
            colorAttachments_.push_back(renderTarget);
            colorAttachmentCount_++;
            break;
        case FBAttachment::DepthStencil:
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, cubemap, 0);
            depthStencilAttachment_ = renderTarget;
            break;
        case FBAttachment::Depth:
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemap, 0);
            depthAttachment_ = renderTarget;
            break;
    }
    
    glBindFramebuffer(target_, 0);
}

// Framebuffer Texture Generator
unsigned int Framebuffer::CreateFramebufferTexture(FBAttachment attachmentType)
{
    unsigned int fbTex;

    switch(attachmentType)
    {
        case FBAttachment::Color:
            fbTex = GenFramebufferTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
            break;
        case FBAttachment::ColorHDR:
            fbTex = GenFramebufferTexture(GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE);
            break;
        case FBAttachment::DepthStencil:
            fbTex = GenFramebufferTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
            break;
        case FBAttachment::Depth:
            fbTex = GenFramebufferTexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
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
        case FBAttachment::ColorHDR:
            renderbuffer = GenRenderbuffer(GL_RGB16F);
            break;
        case FBAttachment::DepthStencil:
            renderbuffer = GenRenderbuffer(GL_DEPTH24_STENCIL8);
            break;
        case FBAttachment::Depth:
            renderbuffer = GenRenderbuffer(GL_DEPTH_COMPONENT);
            break;
    }

    return renderbuffer;
}

unsigned int Framebuffer::CreateCubemapTexture(FBAttachment attachmentType)
{
    unsigned int cubemapTex;
    
    switch(attachmentType)
    {
        case FBAttachment::Color:
            cubemapTex = GenCubemapTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
            break;
        case FBAttachment::ColorHDR:
            cubemapTex = GenCubemapTexture(GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE);
            break;
        case FBAttachment::DepthStencil:
            cubemapTex = GenCubemapTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
            break;
        case FBAttachment::Depth:
            cubemapTex = GenCubemapTexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
            break;
    }
    
    return cubemapTex;
}

unsigned int Framebuffer::GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    
    bool multisampled = samples_ > 1;
    auto sampleType = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    
    glBindTexture(sampleType, texture);
    
    if (multisampled)
    {
        glTexImage2DMultisample(sampleType, samples_, internalFormat, width_, height_, GL_TRUE);
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

unsigned int Framebuffer::GenRenderbuffer(GLenum internalFormat)
{
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    if (samples_ == 1)
    {
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width_, height_);
    }
    else
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, internalFormat, width_, height_);
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rbo;
}

unsigned int Framebuffer::GenCubemapTexture(GLint internalFormat, GLenum format, GLenum dataType)
{
    unsigned int cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width_, height_, 0, format, dataType, nullptr);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return cubemap;
}
