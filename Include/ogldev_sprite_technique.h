/*
    Copyright 2022 Etay Meiri

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

#ifndef SPRITE_TECHNIQUE_H
#define SPRITE_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

#define SPRITE_TECH_MAX_QUADS 100

class SpriteTechnique : public Technique
{
public:

    SpriteTechnique();

    virtual bool Init();

    void SetTextureUnit(unsigned int TextureUnit);

    void SetQuad(int Index,
                 float NDCX, float NDCY, float Width, float Height,    // tile base position and dimensions
                 float u, float v, float TexWidth, float TexHeight);   // texture coordinates

    void UpdateProgram();

private:

    void ConfigUniformBlock();

    struct {
        GLuint BasePos        = -1;
        GLuint WidthHeight    = -1;
        GLuint TexCoords      = -1;
        GLuint TexWidthHeight = -1;
    } m_quadsLoc[SPRITE_TECH_MAX_QUADS];

    struct {
        GLuint BasePos        = -1;
        GLuint WidthHeight    = -1;
        GLuint TexCoords      = -1;
        GLuint TexWidthHeight = -1;
    } m_quads1Loc;

    struct {
        GLint BasePos        = 0;
        GLint WidthHeight    = 0;
        GLint TexCoords      = 0;
        GLint TexWidthHeight = 0;
    } m_quadInfoOffsets;

    GLuint m_samplerLoc = -1;
    GLubyte* m_quadInfoBuffer = NULL;
    GLuint m_uniformBuffer = 0;
    GLint m_blockSize = 0;
};

#endif  /* SPRITE_TECHNIQUE_H */
