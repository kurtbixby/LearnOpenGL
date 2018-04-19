#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "Structs.h"

class Framebuffer
{
public:
	Framebuffer();
    Framebuffer(uint32_t width, uint32_t height);
    Framebuffer(unsigned int width, unsigned int height, uint32_t samples, bool useColorBuffer = true, GLenum target = GL_FRAMEBUFFER);

	void Use();
	void Use(GLenum target);
    
	static void UseDefault(GLenum target = GL_FRAMEBUFFER);

    void SetViewPort();
    void SetSamples(uint32_t samples);
    
	bool IsComplete();
    bool IsMultiSample();
    
    float AspectRatio();
    glm::vec2 Center();
	RenderTarget RetrieveColorBuffer(unsigned int bufferNumber);
	RenderTarget RetrieveDepthBuffer();
	RenderTarget RetrieveDepthStencilBuffer();

    void DownsampleToFramebuffer(Framebuffer& other_fb);
    
	void AddTextureAttachment(FBAttachment attachmentType);
	void AddRenderbufferAttachment(FBAttachment attachmentType);
    void AddCubemapAttachment(FBAttachment attachmentType);

private:
	unsigned int fbo_;
	GLenum target_;
	unsigned int width_;
	unsigned int height_;
    unsigned int samples_;
    bool noColor_;
    bool hasAttachments_;

	unsigned int colorAttachmentCount_;
	std::vector<RenderTarget> colorAttachments_;
	RenderTarget depthAttachment_;
	RenderTarget depthStencilAttachment_;

	unsigned int CreateFramebufferTexture(FBAttachment attachmentType);
	unsigned int CreateRenderBuffer(FBAttachment attachmentType);
    unsigned int CreateCubemapTexture(FBAttachment attachmentType);

	unsigned int GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType);
	unsigned int GenRenderbuffer(GLenum internalFormat);
    unsigned int GenCubemapTexture(GLint internalFormat, GLenum format, GLenum dataType);
};

#endif
