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

#include "shadow_map_technique.h"


static const char* pEffectFile = "shaders/shadow_map.glsl";

ShadowMapTechnique::ShadowMapTechnique(): Technique(pEffectFile)
{
}

bool ShadowMapTechnique::Init()
{
    if (!CompileProgram("ShadowMap")) {
        return false;
    }

    for (uint i = 0 ; i < NUM_OF_LAYERS ; i++)
    {
        char buf[20];
        ZERO_MEM(buf);
        snprintf(buf, sizeof(buf), "gWVP[%d]", i);
        m_WVPLocation[i] = GetUniformLocation(buf);
        if (m_WVPLocation[i] == INVALID_UNIFORM_LOCATION) {            
            return false;
        }        
    }
    
    return true;
}

void ShadowMapTechnique::SetWVP(GLenum Layer, const Matrix4f& WVP)
{
    uint Index;
    
    switch (Layer)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            Index = 0;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            Index = 1;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            Index = 2;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            Index = 3;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            Index = 4;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            Index = 5;
            break;
        default:
            printf("Invalid layer 0x%x\n", Layer);
            exit(0);
    }
    
    glUniformMatrix4fv(m_WVPLocation[Index], 1, GL_TRUE, (const GLfloat*)WVP.m);
}

