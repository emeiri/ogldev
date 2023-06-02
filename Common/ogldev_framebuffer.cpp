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

	if (m_textureBuffer != 0) {
		glDeleteTextures(1, &m_textureBuffer);
	}

	if (m_depthBuffer != 0) {
		glDeleteTextures(1, &m_depthBuffer);
	}
}


void Framebuffer::Init(int Width, int Height)
{
	m_width = Width;
	m_height = Height;

	glGenFramebuffers(1, &m_fbo);

	glGenTextures(1, &m_textureBuffer);
	glBindTexture(GL_TEXTURE_2D, m_textureBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); GLExitIfError;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		exit(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Framebuffer::BindForWriting()
{
	m_saveViewport.Save();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);  // set the width/height of the shadow map!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Framebuffer::UnbindWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	m_saveViewport.Restore();
}


void Framebuffer::BindForReading(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, m_textureBuffer);
}


void Framebuffer::BindDepthForReading(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
}
