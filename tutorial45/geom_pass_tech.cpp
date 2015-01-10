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

#include "geom_pass_tech.h"
#include "ogldev_util.h"


GeomPassTech::GeomPassTech()
{   
}

bool GeomPassTech::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/geometry_pass.vs")) {
        return false;
    }


    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/geometry_pass.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLocation = GetUniformLocation("gWVP");
    m_WVLocation = GetUniformLocation("gWV");


	if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
		m_WVLocation == INVALID_UNIFORM_LOCATION) {
		return false;
	}

	return true;
}


void GeomPassTech::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);    
}


void GeomPassTech::SetWVMatrix(const Matrix4f& WV)
{
    glUniformMatrix4fv(m_WVLocation, 1, GL_TRUE, (const GLfloat*)WV.m);
}


