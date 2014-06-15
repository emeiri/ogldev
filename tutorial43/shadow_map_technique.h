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

#ifndef SHADOW_MAP_TECHNIQUE_H
#define	SHADOW_MAP_TECHNIQUE_H

#include "technique.h"
#include "math_3d.h"
#include "mesh.h"
#include "camera.h"

#define NUM_OF_LAYERS 6

class ShadowMapTechnique : public Technique {

public:

    ShadowMapTechnique();

    virtual bool Init();
    
    void SetWVP(const Matrix4f& WVP);	
    void SetWorld(const Matrix4f& World);	
    void SetLightWorldPos(const Vector3f& Pos);
    
private:

    GLint m_WVPLocation;
    GLint m_WorldMatrixLocation;
    GLint m_lightWorldPosLoc;
};


#endif	/* SHADOW_MAP_TECHNIQUE_H */

