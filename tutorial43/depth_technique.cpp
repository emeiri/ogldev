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

#include <limits.h>
#include <string>
#include <glfx.h>
#include <assert.h>

#include "util.h"
#include "depth_technique.h"

using namespace std;

static const char* pEffectFile = "shaders/depth.glsl";

DepthTechnique::DepthTechnique() : Technique(pEffectFile)
{   
}


bool DepthTechnique::Init()
{
    if (!CompileProgram("Depth")) {
        return false;
    }
    
    m_WVPLocation = GetUniformLocation("gWVP");

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gBones[%d]", i);
        m_boneLocation[i] = GetUniformLocation(Name);
        
        if (m_boneLocation[i] == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    return true;
}


void DepthTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP);    
}



void DepthTechnique::SetBoneTransform(uint Index, const Matrix4f& Transform)
{
    assert(Index < MAX_BONES);
    glUniformMatrix4fv(m_boneLocation[Index], 1, GL_TRUE, (const GLfloat*)Transform);       
}

