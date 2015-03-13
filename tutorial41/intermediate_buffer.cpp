/*
        Copyright 2011 Etay Meiri

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

#include "ogldev_util.h"
#include "ogldev_engine_common.h"
#include "intermediate_buffer.h"

IntermediateBuffer::IntermediateBuffer()
{
    m_fbo = 0;
    m_colorBuffer = 0;
    m_motionBuffer = 0;
	m_depthBuffer = 0;
}


IntermediateBuffer::~IntermediateBuffer()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_colorBuffer != 0) {
        glDeleteTextures(1, &m_colorBuffer);
    }

	if (m_motionBuffer != 0) {
		glDeleteTextures(1, &m_motionBuffer);
	}

	if (m_depthBuffer != 0) {
		glDeleteTextures(1, &m_depthBuffer);
	}	
}


bool IntermediateBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);    
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    // Create the buffers
    glGenTextures(1, &m_colorBuffer);
	glGenTextures(1, &m_motionBuffer);
	glGenTextures(1, &m_depthBuffer);

    // color buffer
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);

    // motion buffer
    glBindTexture(GL_TEXTURE_2D, m_motionBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_motionBuffer, 0);
   
	// depth buffer    
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    
   	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}


void IntermediateBuffer::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}


void IntermediateBuffer::BindForReading()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glActiveTexture(COLOR_TEXTURE_UNIT);		
	glBindTexture(GL_TEXTURE_2D, m_colorBuffer);    
    
	glActiveTexture(MOTION_TEXTURE_UNIT);		
	glBindTexture(GL_TEXTURE_2D, m_motionBuffer);    
}
