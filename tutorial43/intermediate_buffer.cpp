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

#include "util.h"
#include "engine_common.h"
#include "intermediate_buffer.h"

IntermediateBuffer::IntermediateBuffer()
{
    m_fbo = 0;
    m_ambientBuffer = 0;
	m_depthBuffer = 0;
}


IntermediateBuffer::~IntermediateBuffer()
{
    glDeleteFramebuffers(ARRAY_SIZE_IN_ELEMENTS(m_fbos), m_fbos);

    if (m_ambientBuffer != 0) {
        glDeleteTextures(1, &m_ambientBuffer);
    }

	if (m_depthBuffer != 0) {
		glDeleteTextures(1, &m_depthBuffer);
	}	
}


bool IntermediateBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBOs
    glGenFramebuffers(ARRAY_SIZE_IN_ELEMENTS(m_fbos), m_fbos);
        
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[DEPTH_FBO]);

    // Create the buffers
	glGenTextures(1, &m_depthBuffer);

	// depth buffer    
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }
    
    // Create the FBO       
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbos[AMBIENT_FBO]);
    
    // ambient buffer
    glGenTextures(1, &m_ambientBuffer);
    
    glBindTexture(GL_TEXTURE_2D, m_ambientBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ambientBuffer, 0);
    
   	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };

    glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}


void IntermediateBuffer::BindForDepthPass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[DEPTH_FBO]);
}


void IntermediateBuffer::BindForAmbientPass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[AMBIENT_FBO]);

	glActiveTexture(COLOR_TEXTURE_UNIT);		
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);    
}
