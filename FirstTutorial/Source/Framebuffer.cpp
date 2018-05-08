#include "Headers/Framebuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>
#include <algorithm>

//#include "Headers/Debug.h"
#include "Headers/Structs.h"
#include "Headers/TextureCreator.h"

Framebuffer::Framebuffer(): Framebuffer(800, 600, 1, true, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(uint32_t width, uint32_t height): Framebuffer(width, height, 1, true, GL_FRAMEBUFFER) {}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, uint32_t samples, bool useColorBuffer, GLenum target)
{
    width_ = width;
    height_ = height;
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

void Framebuffer::SetBufferClear(uint32_t clearBits, glm::vec4 clearColor, uint32_t stencilClear, float depthClear)
{
    clearBits_ = clearBits;
    colorClearValue_ = clearColor;
    depthClearValue_ = depthClear;
    stencilClearValue_ = stencilClear;
}

void Framebuffer::Clear()
{
    glClearColor(colorClearValue_.r, colorClearValue_.g, colorClearValue_.b, colorClearValue_.a);
    glClearDepth(depthClearValue_);
    glClearStencil(stencilClearValue_);
    glClear(clearBits_);
}

void Framebuffer::Use() const
{
    Use(target_);
}

void Framebuffer::Use(GLenum target) const
{
    glBindFramebuffer(target, fbo_);
    
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

bool Framebuffer::IsComplete() const
{
    glBindFramebuffer(target_, fbo_);
    bool isComplete = glCheckFramebufferStatus(target_) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(target_, 0);

    return isComplete;
}

bool Framebuffer::IsMultiSample() const
{
    return samples_ > 1;
}

float Framebuffer::AspectRatio() const
{
    return float(width_) / float(height_);
}

glm::vec2 Framebuffer::Center() const
{
    return glm::vec2(width_ / 2, height_ / 2);
}

RenderTarget Framebuffer::RetrieveColorBuffer(unsigned int bufferNumber) const
{
    if (colorAttachmentCount_ == 0)
    {
        return RenderTarget {0, RenderTargetType::None};
    }
    unsigned int index = std::min(bufferNumber, colorAttachmentCount_ - 1);
    
    RenderTarget attachment = colorAttachments_[index];
    
    return attachment;
}

void Framebuffer::DownsampleToFramebuffer(Framebuffer& other_fb) const
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

void Framebuffer::CopyAttachmentToFramebuffer(Framebuffer& other_fb, GLuint srcAttachmentPoint, GLuint dstAttachmentPoint) const
{
    if (!ValidateBufferCopy(srcAttachmentPoint, dstAttachmentPoint, other_fb))
    {
        return;
    }
    GLuint bufferBit = BufferBitForAttachmentPoint(srcAttachmentPoint);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other_fb.fbo_);
    {
        glReadBuffer(srcAttachmentPoint);
        glReadBuffer(dstAttachmentPoint);
        glBlitFramebuffer(0, 0, this->width_, this->height_, 0, 0, other_fb.width_, other_fb.height_, bufferBit, GL_NEAREST);
    }
}

RenderTarget Framebuffer::RetrieveDepthBuffer() const
{
    return depthAttachment_;
}

RenderTarget Framebuffer::RetrieveDepthStencilBuffer() const
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
        case FBAttachment::DepthHiRes:
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
        case FBAttachment::DepthHiRes:
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
        case FBAttachment::DepthHiRes:
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

    TextureOptions options = ConfigureTextureOptions(attachmentType);
    switch(attachmentType)
    {
        case FBAttachment::Color:
            fbTex = TextureCreator::CreateTexture(options, width_, height_, samples_);
            break;
        case FBAttachment::ColorHDR:
            fbTex = TextureCreator::CreateTexture(options, width_, height_, samples_);
            break;
        case FBAttachment::DepthStencil:
            fbTex = TextureCreator::CreateTexture(options, width_, height_, samples_);
            break;
        case FBAttachment::DepthHiRes:
        case FBAttachment::Depth:
            fbTex = TextureCreator::CreateTexture(options, width_, height_, samples_);
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
        case FBAttachment::DepthHiRes:
            renderbuffer = GenRenderbuffer(GL_DEPTH_COMPONENT32);
            break;
    }

    return renderbuffer;
}

unsigned int Framebuffer::CreateCubemapTexture(FBAttachment attachmentType)
{
    unsigned int cubemapTex;
    
    TextureOptions options = ConfigureTextureOptions(attachmentType);
    // Overwrite to cubemap config
    options.MinFilter = GL_NEAREST;
    options.MagFilter = GL_NEAREST;
    options.WrapType = GL_CLAMP_TO_EDGE;
    
    switch(attachmentType)
    {
        case FBAttachment::Color:
            cubemapTex = TextureCreator::CreateCubemapTexture(options, width_, height_);
            break;
        case FBAttachment::ColorHDR:
            cubemapTex = TextureCreator::CreateCubemapTexture(options, width_, height_);
            break;
        case FBAttachment::DepthStencil:
            cubemapTex = TextureCreator::CreateCubemapTexture(options, width_, height_);
            break;
        case FBAttachment::DepthHiRes:
        case FBAttachment::Depth:
            cubemapTex = TextureCreator::CreateCubemapTexture(options, width_, height_);
            break;
    }
    
    return cubemapTex;
}

TextureOptions Framebuffer::ConfigureTextureOptions(FBAttachment attachmentType)
{
    TextureOptions options;
    switch (attachmentType)
    {
        case FBAttachment::Color:
            options.TexFormat = GL_RGB;
            options.SourceDataFormat = GL_RGB;
            options.SourceDataType = GL_UNSIGNED_BYTE;
            options.MinFilter = GL_LINEAR;
            options.MagFilter = GL_LINEAR;
            options.WrapType = GL_CLAMP_TO_EDGE;
            break;
        case FBAttachment::ColorHDR:
            options.TexFormat = GL_RGB16F;
            options.SourceDataFormat = GL_RGB;
            options.SourceDataType = GL_FLOAT;
            options.MinFilter = GL_LINEAR;
            options.MagFilter = GL_LINEAR;
            options.WrapType = GL_CLAMP_TO_EDGE;
            break;
        case FBAttachment::DepthStencil:
            options.TexFormat = GL_DEPTH24_STENCIL8;
            options.SourceDataFormat = GL_DEPTH_STENCIL;
            options.SourceDataType = GL_UNSIGNED_INT_24_8;
            options.MinFilter = GL_NEAREST;
            options.MagFilter = GL_NEAREST;
            options.WrapType = GL_CLAMP_TO_BORDER;
            break;
        case FBAttachment::Depth:
            options.TexFormat = GL_DEPTH_COMPONENT;
            options.SourceDataFormat = GL_DEPTH_COMPONENT;
            options.SourceDataType = GL_FLOAT;
            options.MinFilter = GL_NEAREST;
            options.MagFilter = GL_NEAREST;
            options.WrapType = GL_CLAMP_TO_BORDER;
            break;
        case FBAttachment::DepthHiRes:
            options.TexFormat = GL_DEPTH_COMPONENT16;
            options.SourceDataFormat = GL_DEPTH_COMPONENT;
            options.SourceDataType = GL_FLOAT;
            options.MinFilter = GL_NEAREST;
            options.MagFilter = GL_NEAREST;
            options.WrapType = GL_CLAMP_TO_BORDER;
            break;
    }
    return options;
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

bool Framebuffer::ValidateBufferCopy(GLuint srcAttachmentPoint, GLuint dstAttachmentPoint, Framebuffer& otherFb) const
{
    if (CheckAttachmentPointType(srcAttachmentPoint) != CheckAttachmentPointType(dstAttachmentPoint) || srcAttachmentPoint == 0)
    {
        // Different types
        return false;
    }
    
    // Ensure both buffers have the attachment
    if (!HasAttachment(srcAttachmentPoint) || !otherFb.HasAttachment(dstAttachmentPoint))
    {
        return false;
    }
    return true;
}

GLuint Framebuffer::CheckAttachmentPointType(GLuint attachmentPoint)
{
    if (GL_COLOR_ATTACHMENT0 <= attachmentPoint && attachmentPoint <= GL_COLOR_ATTACHMENT31)
    {
        return GL_COLOR_ATTACHMENT0;
    }
    if (GL_DEPTH_ATTACHMENT == attachmentPoint)
    {
        return GL_DEPTH_ATTACHMENT;
    }
    if (GL_STENCIL_ATTACHMENT == attachmentPoint)
    {
        return GL_STENCIL_ATTACHMENT;
    }
    if (GL_DEPTH_STENCIL_ATTACHMENT == attachmentPoint)
    {
        return GL_DEPTH_STENCIL_ATTACHMENT;
    }
    return 0;
}

bool Framebuffer::HasAttachment(GLuint attachmentPoint) const
{
    if (attachmentPoint == GL_DEPTH_ATTACHMENT)
    {
        return depthAttachment_.TargetType != RenderTargetType::None;
    }
    if (attachmentPoint == GL_DEPTH_STENCIL_ATTACHMENT)
    {
        return depthStencilAttachment_.TargetType != RenderTargetType::None;
    }
#warning CHANGE IF STENCIL ATTACHMENT IS IMPLEMENTED
    if (attachmentPoint == GL_STENCIL_ATTACHMENT)
    {
        return false;
    }
    if (GL_COLOR_ATTACHMENT0 <= attachmentPoint && attachmentPoint <= GL_COLOR_ATTACHMENT31)
    {
        // Is color attachment
        if (colorAttachmentCount_ == 0)
        {
            // No color attachments
            return false;
        }
        uint32_t attachmentNumber = attachmentPoint - GL_COLOR_ATTACHMENT0;
        return attachmentNumber <= colorAttachmentCount_ - 1;
    }
    return false;
}

GLuint Framebuffer::BufferBitForAttachmentPoint(GLuint attachmentPoint)
{
    if (GL_COLOR_ATTACHMENT0 <= attachmentPoint && attachmentPoint <= GL_COLOR_ATTACHMENT31)
    {
        return GL_COLOR_BUFFER_BIT;
    }
    if (GL_DEPTH_ATTACHMENT == attachmentPoint)
    {
        return GL_DEPTH_BUFFER_BIT;
    }
    if (GL_STENCIL_ATTACHMENT == attachmentPoint)
    {
        return GL_STENCIL_BUFFER_BIT;
    }
    if (GL_DEPTH_STENCIL_ATTACHMENT == attachmentPoint)
    {
        return GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    }
    return 0;
}
