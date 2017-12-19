#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

#include "Structs.h"

class Framebuffer
{
public:
	Framebuffer();
	Framebuffer(unsigned int width, unsigned int height, GLenum target = GL_FRAMEBUFFER);

	void Use();
	void Use(GLenum target);

	bool IsComplete();
	RenderTarget RetrieveColorBuffer(unsigned int bufferNumber);
	RenderTarget RetrieveDepthBuffer();
	RenderTarget RetrieveStencilBuffer();
	RenderTarget RetrieveDepthStencilBuffer();

	void AddTextureAttachment(FBAttachment attachmentType);
	void AddRenderbufferAttachment(FBAttachment attachmentType);

private:
	unsigned int fbo_;
	GLenum target_;
	unsigned int width_;
	unsigned int height_;

	unsigned int colorAttachmentCount_;
	std::vector<RenderTarget> colorAttachments_;
	RenderTarget depthAttachment_;
	RenderTarget stencilAttachment_;
	RenderTarget depthStencilAttachment_;

	unsigned int CreateFramebufferTexture(FBAttachment attachmentType);
	unsigned int CreateRenderBuffer(FBAttachment attachmentType);

	unsigned int GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType);
	unsigned int GenRenderbuffer(GLenum internalFormat);
};

#endif
