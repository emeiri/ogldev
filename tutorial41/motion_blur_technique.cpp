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


#include "motion_blur_technique.h"
#include "ogldev_util.h"

MotionBlurTechnique::MotionBlurTechnique()
{
    
}

bool MotionBlurTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/motion_blur.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/motion_blur.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }
    
    m_colorTextureLocation = GetUniformLocation("gColorTexture");
    m_motionTextureLocation = GetUniformLocation("gMotionTexture");
    
    if (m_motionTextureLocation == INVALID_UNIFORM_LOCATION ||
        m_colorTextureLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

	return true;
}


void MotionBlurTechnique::SetColorTextureUnit(uint TextureUnit)
{
    glUniform1i(m_colorTextureLocation, TextureUnit);
}


void MotionBlurTechnique::SetMotionTextureUnit(uint TextureUnit)
{
    glUniform1i(m_motionTextureLocation, TextureUnit);
}

