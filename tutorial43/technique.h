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

#ifndef TECHNIQUE_H
#define	TECHNIQUE_H

#include <list>
#include <GL/glew.h>

#include "math_3d.h"
#include "mesh.h"
#include "camera.h"

class Technique
{
public:

    Technique(const char* pEffectFile);

    ~Technique();

    void Enable();

    void SetWVP(const Matrix4f& WVP);
    
	void SetWV(const Matrix4f& WV);
    
    void SetWorldMatrix(const Matrix4f& WVP);
 
    virtual bool Init();
    
    void ApplyOrientation(const Orientation& orientation, 
                          const Vector3f& CameraPos, 
                          const Vector3f& CameraTarget, 
                          const Vector3f& CameraUp, 
                          PersProjInfo& ProjInfo);
    
protected:
    
    bool CompileProgram(const char* pProgram);
    
    GLint GetUniformLocation(const char* pUniformName);
    
    GLint GetProgramParam(GLint param);
    
    GLuint m_WVPLocation;
	GLuint m_WVLocation;
    GLuint m_WorldMatrixLocation;

private:    
    GLint m_effect;    
    GLint m_shaderProg;
    const char* m_pEffectFile;
};

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF


#endif	/* TECHNIQUE_H */

