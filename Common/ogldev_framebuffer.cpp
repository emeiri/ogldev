#include <stdlib.h>
#include <stdio.h>

#include "ogldev_util.h"
#include "ogldev_framebuffer.h"


Framebuffer::Framebuffer()
{

}


Framebuffer::~Framebuffer()
{
	if (m_fbo != 0) {
		glDeleteFramebuffers(1, &m_fbo);
	}

	if (m_colorBuffer != 0) {
		glDeleteTextures(1, &m_colorBuffer);
	}

	if (m_depthBuffer != 0) {
		glDeleteTextures(1, &m_depthBuffer);
	}
}


void Framebuffer::Init(int Width, int Height, int NumFormatComponents, bool DepthEnabled)
{
	m_width = Width;
	m_height = Height;

	glGenFramebuffers(1, &m_fbo);

	GenerateColorBuffer(Width, Height, NumFormatComponents);

	if (DepthEnabled) {
		GenerateDepthBuffer(Width, Height);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);

	if (DepthEnabled) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
	}	

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		exit(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Framebuffer::GenerateDepthBuffer(int Width, int Height)
{
	glGenTextures(1, &m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); GLExitIfError;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Framebuffer::GenerateColorBuffer(int Width, int Height, int NumFormatComponents)
{
	glGenTextures(1, &m_colorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffer);

	switch (NumFormatComponents) {
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		break;
	
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		break;

	default:
		printf("Unsupported num components %d\n", NumFormatComponents);
		exit(1);
	}
		
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void Framebuffer::BindForWriting()
{
	m_saveViewport.Save();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);  
}


void Framebuffer::UnbindWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	m_saveViewport.Restore();
}


void Framebuffer::BindForReading(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
}


void Framebuffer::BindDepthForReading(GLenum TextureUnit)
{
	if (m_depthBuffer == -1) {
		printf("Trying to bind depth for reading in a FBO without depth buffer\n");
		exit(1);
	}

	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
}


void Framebuffer::ClearColorBuffer(const Vector4f& Color)
{
	glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, (GLfloat*)Color.data());
}
