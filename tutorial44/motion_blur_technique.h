/*

	Copyright 2013 Etay Meiri

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

#ifndef MOTION_BLUR_TECHNIQUE_H
#define	MOTION_BLUR_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

class MotionBlurTechnique : public Technique {
public:

    MotionBlurTechnique();

    virtual bool Init();

    void SetColorTextureUnit(uint TextureUnit);    
    void SetMotionTextureUnit(uint TextureUnit);
    
private:
    GLuint m_colorTextureLocation;
    GLuint m_motionTextureLocation;
};



#endif	/* MOTION_BLUR_TECHNIQUE_H */

