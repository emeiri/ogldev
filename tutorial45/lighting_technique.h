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

    void SetPositionTextureUnit(uint TextureUnit);	
    void SetNormalTextureUnit(uint TextureUnit);	
    void SetRandomTextureUnit(uint TextureUnit);
    void SetDepthTextureUnit(uint TextureUnit);
    void SetScreenSize(uint Width, uint Height);
    void SetAmbientIntensity(float ai);    
    void SetSampleRadius(float sr);    
    void SetProjMatrix(const Matrix4f& m);
    
private:
    
    void GenKernel();
    
    const static uint KERNEL_SIZE = 128;

    GLuint m_posTextureUnitLocation;
    GLuint m_normalTextureUnitLocation;
    GLuint m_randomTextureUnitLocation;
    GLuint m_depthTextureUnitLocation;
    GLuint m_screenSizeLocation;
    GLuint m_AILocation;    
    GLuint m_sampleRadLocation;    
    GLuint m_kernelLocation;
    GLuint m_projMatrixLocation;
};


#endif	/* LIGHTING_TECHNIQUE_H */
