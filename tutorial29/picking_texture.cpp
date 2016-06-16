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

#include "picking_texture.h"
#include "picking_technique.h"
#include "ogldev_util.h"

PickingTexture::PickingTexture()
{
    m_fbo = 0;
    m_pickingTexture = 0;
    m_depthTexture = 0;
}

PickingTexture::~PickingTexture()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_pickingTexture != 0) {
        glDeleteTextures(1, &m_pickingTexture);
    }
    
    if (m_depthTexture != 0) {
        glDeleteTextures(1, &m_depthTexture);
    }
}


bool PickingTexture::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);    
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create the texture object for the primitive information buffer
    glGenTextures(1, &m_pickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pickingTexture, 0);    

    // Create the texture object for the depth buffer
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);    

	glReadBuffer(GL_NONE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Verify that the FBO is correct
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }
    
    // Restore the default framebuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return GLCheckError();
}


void PickingTexture::EnableWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}


void PickingTexture::DisableWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


PickingTexture::PixelInfo PickingTexture::ReadPixel(unsigned int x, unsigned int y)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    PixelInfo Pixel;
    glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &Pixel);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return Pixel;
}