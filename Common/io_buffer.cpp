/*
        Copyright 2015 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_io_buffer.h"
#include "ogldev_texture.h"


IOBuffer::IOBuffer()
{
    m_fbo          = 0;
    m_texture      = 0;
    m_depth        = 0;
    m_internalType = GL_NONE;
}

IOBuffer::~IOBuffer()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
    }

    if (m_depth != 0) {
        glDeleteTextures(1, &m_depth);
    }
}


bool IOBuffer::Init(uint WindowWidth, uint WindowHeight, bool WithDepth, GLenum InternalType)
{
    m_internalType = InternalType;
    
    GLenum Format, Type;
    
    switch (InternalType) {
        case GL_RGB32F:
            Format = GL_RGB;
            Type = GL_FLOAT;
            break;
        case GL_R32F:
            Format = GL_RED;
            Type = GL_FLOAT;
            break;
        case GL_NONE:
            break;
        default:
            OGLDEV_ERROR0("Invalid internal type");
    }
    
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);    
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create the textures
    if (InternalType != GL_NONE) {
        glGenTextures(1, &m_texture);

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalType, WindowWidth, WindowHeight, 0, Format, Type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };

        glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);
    }
        
    // Create the depth buffer 
    if (WithDepth) {
        glGenTextures(1, &m_depth);

        // depth
        glBindTexture(GL_TEXTURE_2D, m_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);        
    }
    
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}


void IOBuffer::BindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}


void IOBuffer::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    
    if (m_internalType == GL_NONE) {
        glBindTexture(GL_TEXTURE_2D, m_depth);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }
}




