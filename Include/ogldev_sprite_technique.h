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
                 float x, float y, float Width, float Height,
                 float u, float v, float TexWidth, float TexHeight);

private:

    GLuint m_colorLoc = -1;


    struct {
        GLuint BasePos        = -1;
        GLuint WidthHeight    = -1;
        GLuint TexCoords      = -1;
        GLuint TexWidthHeight = -1;
    } m_quadsLoc[SPRITE_TECH_MAX_QUADS];

    GLuint m_samplerLoc = -1;
};

#endif  /* SPRITE_TECHNIQUE_H */
