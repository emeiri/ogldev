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
#include <bmpfile.h>


#include "shadow_map_fbo.h"
#include "util.h"
#include "shadow_map_technique.h"

ShadowMapFBO::ShadowMapFBO()
{
    m_fbo = 0;
    m_shadowMap = 0;
}

ShadowMapFBO::~ShadowMapFBO()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_shadowMap != 0) {
        glDeleteTextures(1, &m_shadowMap);
    }
}

bool ShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
    
    for (uint i = 0 ; i < 6 ; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_shadowMap, 0);

    // Disable writes to the color buffer
    glDrawBuffer(GL_NONE);
       
    // Disable reads from the color buffer
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    return true;
}


void ShadowMapFBO::BindForWriting(GLenum CubeFace)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CubeFace, m_shadowMap, 0);
}


void ShadowMapFBO::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}


void ShadowMapFBO::Dump()
{
    float Pixels[1000][1000];
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
    
    bmpfile_t* pBMP = bmp_create(1000, 1000, 4);
    
    if (!pBMP) {
        printf("Error creating bmp\n");
        exit(0);
    }
    
    for (int j = 0 ; j < NUM_OF_LAYERS ; j++) {
        ZERO_MEM(Pixels);
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, GL_FLOAT, Pixels);
        GLExitIfError();
       
        char filename[10];
        snprintf(filename, sizeof(filename), "face%d.bmp", j);       
        
        for (int y = 0 ; y < 1000 ; y++ ) {
            for (int x = 0 ; x < 1000 ; x++) {
                float f;
                if (Pixels[x][y] == 1.0f)
                    f = 0.0f;
                else
                    f = Pixels[x][y] * 255.0f;                
                rgb_pixel_t pixel;
                pixel.red = f;
                pixel.green = f;
                pixel.blue = f;
                
                bmp_set_pixel(pBMP, x, y, pixel);
            }
        }
        
        bmp_save(pBMP, filename);       
    }
    
    
}