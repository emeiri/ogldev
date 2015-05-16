/*

	Copyright 2015 Etay Meiri

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

#include "blur_tech.h"
#include "ogldev_util.h"

#define INPUT_TEXTURE_UNIT                 GL_TEXTURE0
#define INPUT_TEXTURE_UNIT_INDEX           0


BlurTech::BlurTech()
{   
}

bool BlurTech::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/blur.vs")) {
        return false;
    }


    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/blur.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_inputTextureUnitLocation = GetUniformLocation("gColorMap");

	if (m_inputTextureUnitLocation == INVALID_UNIFORM_LOCATION) {
		return false;
	}
    
    Enable();
    
    glUniform1i(m_inputTextureUnitLocation, INPUT_TEXTURE_UNIT_INDEX);

	return true;
}


void BlurTech::BindInputBuffer(IOBuffer& inputBuf)
{
    inputBuf.BindForReading(INPUT_TEXTURE_UNIT);
}

