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

#include "ds_point_light_pass_tech.h"
#include "ogldev_util.h"


DSPointLightPassTech::DSPointLightPassTech()
{   
}

bool DSPointLightPassTech::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/light_pass.vs")) {
        return false;
    }


    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/point_light_pass.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }
   
    m_pointLightLocation.Color = GetUniformLocation("gPointLight.Base.Color");
    m_pointLightLocation.AmbientIntensity = GetUniformLocation("gPointLight.Base.AmbientIntensity");
    m_pointLightLocation.Position = GetUniformLocation("gPointLight.Position");
    m_pointLightLocation.DiffuseIntensity = GetUniformLocation("gPointLight.Base.DiffuseIntensity");
    m_pointLightLocation.Atten.Constant = GetUniformLocation("gPointLight.Atten.Constant");
    m_pointLightLocation.Atten.Linear = GetUniformLocation("gPointLight.Atten.Linear");
    m_pointLightLocation.Atten.Exp = GetUniformLocation("gPointLight.Atten.Exp");

	if (m_pointLightLocation.Color == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.Position == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.Atten.Constant == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.Atten.Linear == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.Atten.Exp == INVALID_UNIFORM_LOCATION) {    
        return false;
    }

	return DSLightPassTech::Init();
}


void DSPointLightPassTech::SetPointLight(const PointLight& Light)
{
    glUniform3f(m_pointLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(m_pointLightLocation.AmbientIntensity, Light.AmbientIntensity);
    glUniform1f(m_pointLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
    glUniform3f(m_pointLightLocation.Position, Light.Position.x, Light.Position.y, Light.Position.z);
    glUniform1f(m_pointLightLocation.Atten.Constant, Light.Attenuation.Constant);
    glUniform1f(m_pointLightLocation.Atten.Linear, Light.Attenuation.Linear);
    glUniform1f(m_pointLightLocation.Atten.Exp, Light.Attenuation.Exp);
}


