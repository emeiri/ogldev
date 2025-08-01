/*
        Copyright 2023 Etay Meiri

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

#include "ogldev_save_viewport.h"
#include "ogldev_math_3d.h"

// TODO: try to merge with the Framebuffer class
class Framebuffer 
{

public:
    Framebuffer();

    ~Framebuffer();

    void Init(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled);

    void BindForWriting();

    void UnbindWriting();

    void BindForReading(GLenum TextureUnit);

    void BindDepthForReading(GLenum TextureUnit);

    void Clear(bool ColorBuffer, bool DepthBuffer);

    void ClearColorBuffer(const Vector4f& Color);

    void BlitToWindow();

    GLuint GetTexture() const { return m_colorBuffer; }

    GLuint GetDepthTexture() const { return m_depthBuffer; }

    int GetWidth() const { return m_width; }

    int GetHeight() const { return m_height; }

private:
    void InitDSA(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled);

    void InitNonDSA(int Width, int Height, int NumFormatComponents, bool IsFloat, bool DepthEnabled);

    void GenerateColorBuffer(int Width, int Height, int NumFormatComponents, bool IsFloat);

    void GenerateColorBufferNonDSA(int NumFormatComponents, bool IsFloat, int Width, int Height);

    void GenerateColorBufferDSA(int NumFormatComponents, bool IsFloat, int Width, int Height);

    void GenerateDepthBuffer(int Width, int Height);

    int m_width = 0;
    int m_height = 0;
    GLuint m_fbo = -1;
    GLuint m_colorBuffer = -1;
    GLuint m_depthBuffer = -1;
    SaveViewport m_saveViewport;
};
