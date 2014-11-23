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
#include <string.h>

#include "ogldev_math_3d.h"
#include "ogldev_util.h"
#include "lighting_technique.h"


LightingTechnique::LightingTechnique()
{   
}

bool LightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/lighting.vs")) {
        return false;
    }


    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

	m_posTextureUnitLocation = GetUniformLocation("gPositionMap");
	m_normalTextureUnitLocation = GetUniformLocation("gNormalMap");
    m_screenSizeLocation = GetUniformLocation("gScreenSize");	

    if (m_posTextureUnitLocation == INVALID_UNIFORM_LOCATION ||		
		m_normalTextureUnitLocation == INVALID_UNIFORM_LOCATION ||		
        m_screenSizeLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void LightingTechnique::SetPositionTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_posTextureUnitLocation, TextureUnit);
}


void LightingTechnique::SetNormalTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_normalTextureUnitLocation, TextureUnit);
}


void LightingTechnique::SetScreenSize(unsigned int Width, unsigned int Height)
{
    glUniform2f(m_screenSizeLocation, (float)Width, (float)Height);
}