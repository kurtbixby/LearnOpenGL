#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "Structs.h"
#include "TextureCreator.h"

class Framebuffer
{
public:
	Framebuffer();
    Framebuffer(uint32_t width, uint32_t height);
    Framebuffer(unsigned int width, unsigned int height, uint32_t samples, bool useColorBuffer = true, GLenum target = GL_FRAMEBUFFER);
    
    void SetBufferClear(uint32_t clearBits, glm::vec4 clearColor = glm::vec4(glm::vec3(0.0f), 1.0f), uint32_t stencilClear = 0x0, float depthClear = 1.0f);
    void Clear();
    
	void Use() const;
	void Use(GLenum target) const;
    
	static void UseDefault(GLenum target = GL_FRAMEBUFFER);

    void SetViewPort();
    void SetSamples(uint32_t samples);
    
	bool IsComplete() const;
    bool IsMultiSample() const;
    
    float AspectRatio() const;
    glm::vec2 Center() const;
	RenderTarget RetrieveColorBuffer(unsigned int bufferNumber) const;
	RenderTarget RetrieveDepthBuffer() const;
	RenderTarget RetrieveDepthStencilBuffer() const;

    void DownsampleToFramebuffer(Framebuffer& other_fb) const;
    void CopyAttachmentToFramebuffer(Framebuffer& other_fb, GLuint srcAttachmentPoint, GLuint dstAttachmentPoint) const;
    
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

    uint32_t clearBits_;
    glm::vec4 colorClearValue_;
    float depthClearValue_;
    uint32_t stencilClearValue_;
    
	unsigned int colorAttachmentCount_;
	std::vector<RenderTarget> colorAttachments_;
	RenderTarget depthAttachment_;
	RenderTarget depthStencilAttachment_;

	unsigned int CreateFramebufferTexture(FBAttachment attachmentType);
	unsigned int CreateRenderBuffer(FBAttachment attachmentType);
    unsigned int CreateCubemapTexture(FBAttachment attachmentType);
    
    TextureOptions ConfigureTextureOptions(FBAttachment attachmentType);

	unsigned int GenRenderbuffer(GLenum internalFormat);
    bool ValidateBufferCopy(GLuint srcAttachmentPoint, GLuint dstAttachmentPoint, Framebuffer& otherFb) const;
    static GLuint CheckAttachmentPointType(GLuint attachmentPoint);
    static GLuint BufferBitForAttachmentPoint(GLuint attachmentPoint);
    bool HasAttachment(GLuint attachmentPoint) const;
};

#endif
