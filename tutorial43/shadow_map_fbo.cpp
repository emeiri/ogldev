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
#include "bmpfile.h"


#include "shadow_map_fbo.h"
#include "util.h"
#include "shadow_map_technique.h"

ShadowMapFBO::ShadowMapFBO()
{
    m_fbo = 0;
    m_depth = 0;
}

ShadowMapFBO::~ShadowMapFBO()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_depth != 0) {
        glDeleteTextures(1, &m_depth);
    }
}

bool ShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    glGenTextures(1, &m_depth);
    glBindTexture(GL_TEXTURE_2D, m_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (uint i = 0 ; i < 6 ; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RED, WindowWidth, WindowHeight, 0, GL_RED, GL_FLOAT, NULL);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_shadowMap, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    GLExitIfError();
       
    glReadBuffer(GL_NONE);
    
    GLExitIfError();

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return GLCheckError();
}


void ShadowMapFBO::BindForWriting(GLenum CubeFace)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CubeFace, m_shadowMap, 0);
}


void ShadowMapFBO::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
}


void ShadowMapFBO::ShowColorBuffer(GLenum CubeFace)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    GLExitIfError();
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CubeFace, m_shadowMap, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1000, 1000, 0, 0, 1000, 1000, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glReadBuffer(GL_NONE);
    GLExitIfError();       
}
/*void ShadowMapFBO::BindForReading(GLenum TextureUnit)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);    
    GLExitIfError();        
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_shadowMap, 0);    
    GLExitIfError();        

	float* pPixels = new float[1000000];
	memset(pPixels, 0, 4000000);
    
    glReadPixels(0, 0, 1000, 1000, GL_RED, GL_FLOAT, pPixels);
    GLExitIfError();
    
    bmpfile_t* pBMP = bmp_create(1000, 1000, 4);
    
    if (!pBMP) {
        printf("Error creating bmp\n");
        exit(0);
    }

	float fMax = 0.0f;

	for (int y = 0 ; y < 1000 ; y++ ) {
		for (int x = 0 ; x < 1000 ; x++) {
			float f = pPixels[y * 1000 + x];
			if (f > fMax) fMax = f;
		}
	}



    for (int y = 0 ; y < 1000 ; y++ ) {
        for (int x = 0 ; x < 1000 ; x++) {
            float f = pPixels[y * 1000 + x];
			int t = (int)(f / fMax * 16777216.0f);
        //    if (pPixels[y * 1000 + x] == 1.0f)
        //        f = 0.0f;
        //    else
          //      f = 1.0f;//Pixels[x][y] * 255.0f;                
            rgb_pixel_t pixel;
            pixel.red = t >> 16;
            pixel.green = t >> 8;
            pixel.blue = t;

            bmp_set_pixel(pBMP, x, y, pixel);
        }
    }
        
    bmp_save(pBMP, "depth.bmp");       

	delete [] pPixels;
}*/

