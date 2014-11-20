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

#ifndef LIGHTING_TECHNIQUE_H
#define	LIGHTING_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"


class LightingTechnique : public Technique {
public:

    LightingTechnique();

    virtual bool Init();

    void SetPositionTextureUnit(unsigned int TextureUnit);	
    void SetNormalTextureUnit(unsigned int TextureUnit);	
    void SetScreenSize(unsigned int Width, unsigned int Height);
    
private:

    GLuint m_posTextureUnitLocation;
    GLuint m_normalTextureUnitLocation;
    GLuint m_screenSizeLocation;
};


#endif	/* LIGHTING_TECHNIQUE_H */
