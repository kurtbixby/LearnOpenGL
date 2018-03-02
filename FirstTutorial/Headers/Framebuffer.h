#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

#include "Structs.h"

class Framebuffer
{
public:
	Framebuffer();
    Framebuffer(unsigned int width, unsigned int height, bool useColorBuffer = true, GLenum target = GL_FRAMEBUFFER);

	void Use();
	void Use(GLenum target);
    
//    unsigned int Width();
//    unsigned int Height();

	static void UseDefault(GLenum target = GL_FRAMEBUFFER);

    void SetViewPort();
    
	bool IsComplete();
	RenderTarget RetrieveColorBuffer(unsigned int bufferNumber);
	RenderTarget RetrieveDepthBuffer();
	RenderTarget RetrieveDepthStencilBuffer();

    void DownsampleToFramebuffer(Framebuffer& other_fb);
    
	void AddTextureAttachment(FBAttachment attachmentType, uint32_t samples = 1);
	void AddRenderbufferAttachment(FBAttachment attachmentType, uint32_t samples = 1);

private:
	unsigned int fbo_;
	GLenum target_;
	unsigned int width_;
	unsigned int height_;
    bool noColor_;

	unsigned int colorAttachmentCount_;
	std::vector<RenderTarget> colorAttachments_;
	RenderTarget depthAttachment_;
	RenderTarget depthStencilAttachment_;

	unsigned int CreateFramebufferTexture(FBAttachment attachmentType, uint32_t samples = 1);
	unsigned int CreateRenderBuffer(FBAttachment attachmentType, uint32_t samples = 1);

	unsigned int GenFramebufferTexture(GLint internalFormat, GLenum format, GLenum dataType, uint32_t samples = 1);
	unsigned int GenRenderbuffer(GLenum internalFormat, uint32_t samples = 1);
};

#endif
