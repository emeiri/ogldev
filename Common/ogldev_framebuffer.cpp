#include <stdlib.h>
#include <stdio.h>

#include "ogldev_util.h"
#include "ogldev_framebuffer.h"


Framebuffer::Framebuffer() {}


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


void Framebuffer::Init(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled)
{
    if (IsGLVersionHigher(4, 5)) {
        InitDSA(Width, Height, NumFormatComponents, IsFloat, DepthEnabled);
    } else {
        InitNonDSA(Width, Height, NumFormatComponents, IsFloat, DepthEnabled);
    }
}


void Framebuffer::InitDSA(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled)
{
    m_width = Width;
    m_height = Height;

    glCreateFramebuffers(1, &m_fbo);

    GenerateColorBuffer(Width, Height, NumFormatComponents, IsFloat);

    if (DepthEnabled) {
        GenerateDepthBuffer(Width, Height);
    }

    glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_colorBuffer, 0);

    if (DepthEnabled) {
        glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);
    }

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffer(m_fbo, drawBuf);
    glNamedFramebufferReadBuffer(m_fbo, drawBuf);

    GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        exit(0);
    }
}


void Framebuffer::InitNonDSA(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled)
{
    m_width = Width;
    m_height = Height;

    glGenFramebuffers(1, &m_fbo);

    GenerateColorBuffer(Width, Height, NumFormatComponents, IsFloat);

    if (DepthEnabled) {
        GenerateDepthBuffer(Width, Height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);

    if (DepthEnabled) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    }

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Framebuffer::GenerateDepthBuffer(int Width, int Height)
{
    if (IsGLVersionHigher(4, 5)) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_depthBuffer);
        glTextureStorage2D(m_depthBuffer, 1, GL_DEPTH_COMPONENT32F, Width, Height);
        glTextureParameteri(m_depthBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_depthBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_depthBuffer, GL_TEXTURE_BASE_LEVEL, 0);
        glTextureParameteri(m_depthBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_depthBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else {
        glGenTextures(1, &m_depthBuffer);
        glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); GLExitIfError;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}


void Framebuffer::GenerateColorBuffer(int Width, int Height, int NumFormatComponents, bool IsFloat)
{
    if (IsGLVersionHigher(4, 5)) {
        GenerateColorBufferDSA(NumFormatComponents, IsFloat, Width, Height);
    }
    else {
        GenerateColorBufferNonDSA(NumFormatComponents, IsFloat, Width, Height);
    }
}


void Framebuffer::GenerateColorBufferDSA(int NumFormatComponents, bool IsFloat, int Width, int Height)
{
    GLenum InternalFormat = 0;

    switch (NumFormatComponents) {
    case 4:
        if (IsFloat) {
            InternalFormat = GL_RGBA32F;
        } else {
            InternalFormat = GL_RGBA8;
        }        
        break;

    case 3:
        if (IsFloat) {
            InternalFormat = GL_RGB16F;
        }
        else {
            InternalFormat = GL_RGB8;
        }        
        break;

    case 1:
        assert(IsFloat);
        InternalFormat = GL_R32F;
        break;

    default:
        assert(0);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_colorBuffer);
    glTextureStorage2D(m_colorBuffer, 1, InternalFormat, Width, Height);
    glTextureParameteri(m_colorBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_colorBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_colorBuffer, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_colorBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_colorBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void Framebuffer::GenerateColorBufferNonDSA(int NumFormatComponents, bool IsFloat, int Width, int Height)
{
    glGenTextures(1, &m_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
    switch (NumFormatComponents) {
    case 4:
        assert(!IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        break;
    case 3:
        assert(!IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        break;
    case 1:
        assert(IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, Width, Height, 0, GL_RED, GL_FLOAT, NULL);
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

void Framebuffer::Clear(bool ColorBuffer, bool DepthBuffer)
{
    assert(ColorBuffer || DepthBuffer);

    Vector4f Color(0.0f);
    float Depth = 1.0f;

    if (IsGLVersionHigher(4, 5)) {
        if (ColorBuffer) {
            glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, (GLfloat*)Color.data());
        }

        if (DepthBuffer) {
            glClearNamedFramebufferfv(m_fbo, GL_DEPTH, 0, &Depth);
        }               
    }
    else {
        if (ColorBuffer) {
            glClearBufferfv(GL_COLOR, 0, (GLfloat*)Color.data());
        }
         
        if (DepthBuffer) {
            glClearBufferfv(GL_DEPTH, 0, &Depth);
        }
    }
}


void Framebuffer::ClearColorBuffer(const Vector4f& Color)
{
    if (IsGLVersionHigher(4, 5)) {
        glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, (GLfloat*)Color.data());
    }
    else {
        glClearBufferfv(GL_COLOR, 0, (GLfloat*)Color.data());
    }
}


void Framebuffer::BlitToWindow()
{
    if (IsGLVersionHigher(4, 5)) {
        glBlitNamedFramebuffer(m_fbo, 0, 0, 0, m_width, m_height, 0, 0, m_width, m_height, 
                               GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
    else {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, 
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}