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

#include <stdio.h>
#include <string>
#include <assert.h>
#include <glfx.h>

#include "technique.h"
#include "util.h"
#include "pipeline.h"

using namespace std;

Technique::Technique(const char* pEffectFile)
{
    m_effectFileName = pEffectFile;
    m_shaderProg = 0;
    m_effect = glfxGenEffect();
}


Technique::~Technique()
{
    if (m_shaderProg != 0)
    {
        glDeleteProgram(m_shaderProg);
        m_shaderProg = 0;
    }
    
    glfxDeleteEffect(m_effect);
}


bool Technique::Init()
{
    m_WVPLocation = GetUniformLocation("gWVP");
	m_WVLocation = GetUniformLocation("gWV");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    
    return true;
}


void Technique::ApplyOrientation(const Orientation& orientation, 
                                 const Vector3f& CameraPos, 
                                 const Vector3f& CameraTarget, 
                                 const Vector3f& CameraUp, 
                                 PersProjInfo& ProjInfo)
{
    Pipeline p;
    
    p.SetPerspectiveProj(ProjInfo);    
    p.SetCamera(CameraPos, CameraTarget, CameraUp);
    p.Scale(orientation.m_scale);
    p.Rotate(orientation.m_rotation);
    p.WorldPos(orientation.m_pos);    
    
    SetWorldMatrix(p.GetWorldTrans());
    SetWV(p.GetWVTrans());
    SetWVP(p.GetWVPTrans());
}


void Technique::ApplyOrientation(const Orientation& orientation, const Camera& camera, PersProjInfo& ProjInfo)
{
    ApplyOrientation(orientation, camera.GetPos(), camera.GetTarget(), camera.GetUp(), ProjInfo);
}


bool Technique::CompileProgram(const char* pProgram)
{
    if (!glfxParseEffectFromFile(m_effect, m_effectFileName.c_str())) {
        string log = glfxGetEffectLog(m_effect);
        printf("Error creating effect from file '%s':\n", m_effectFileName.c_str());
        printf("%s\n", log.c_str());
        return false;
    }
    
    m_shaderProg = glfxCompileProgram(m_effect, pProgram);
    
    if (m_shaderProg < 0) {
        string log = glfxGetEffectLog(m_effect);
        printf("Error compiling program '%s' in effect file '%s':\n", pProgram, m_effectFileName.c_str());
        printf("%s\n", log.c_str());
        return false;
    }
    
    return true;
}

void Technique::Enable()
{
    glUseProgram(m_shaderProg);
}


GLint Technique::GetUniformLocation(const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if (Location == INVALID_OGL_VALUE) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s' in effect file '%s'\n", pUniformName, m_effectFileName.c_str());
    }

    return Location;
}

GLint Technique::GetProgramParam(GLint param)
{
    GLint ret;
    glGetProgramiv(m_shaderProg, param, &ret);
    return ret;
}


void Technique::SetWVP(const Matrix4f& WVP)
{
    if (m_WVPLocation != INVALID_OGL_VALUE) {
        glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
    }
}


void Technique::SetWV(const Matrix4f& WV)
{   
    if (m_WVLocation != INVALID_OGL_VALUE) {
        glUniformMatrix4fv(m_WVLocation, 1, GL_TRUE, (const GLfloat*)WV.m);
    }
}


void Technique::SetWorldMatrix(const Matrix4f& World)
{
    if (m_WorldMatrixLocation != INVALID_OGL_VALUE) {
        glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)World.m);
    }
}

