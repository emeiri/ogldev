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

#pragma once 

#include <GL/glew.h>

// TODO: try to merge with the Framebuffer class
class FramebufferObject
{
public:
    FramebufferObject();

    ~FramebufferObject();

    bool Init(unsigned int Width, unsigned int Height, bool ForPCF = false);

    void BindForWriting();

    void BindForReading(GLenum TextureUnit);

private:

    bool InitNonDSA(unsigned int Width, unsigned int Height, bool ForPCF = false);
    bool InitDSA(unsigned int Width, unsigned int Height, bool ForPCF = false);

    void BindForReadingNonDSA(GLenum TextureUnit);
    void BindForReadingDSA(GLenum TextureUnit);

    uint m_width = 0;
    uint m_height = 0;
    GLuint m_fbo;
    GLuint m_depthBuffer;
};


class CascadedShadowMapFBO
{
public:
    CascadedShadowMapFBO();

    ~CascadedShadowMapFBO();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting(uint CascadeIndex);

    void BindForReading();

private:
    GLuint m_fbo;
    GLuint m_shadowMap[3];
};

