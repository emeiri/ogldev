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
#include "ssao_technique.h"

#define DEPTH_TEXTURE_UNIT           GL_TEXTURE1
#define DEPTH_TEXTURE_UNIT_INDEX     1


SSAOTechnique::SSAOTechnique()
{   
}


bool SSAOTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/ssao.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/ssao.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

	m_depthTextureUnitLocation = GetUniformLocation("gDepthMap");
    m_sampleRadLocation = GetUniformLocation("gSampleRad");
    m_projMatrixLocation = GetUniformLocation("gProj");	
    m_kernelLocation = GetUniformLocation("gKernel");
    m_aspectRatioLocation = GetUniformLocation("gAspectRatio");
    m_tanHalfFOVLocation = GetUniformLocation("gTanHalfFOV");
            
    if (m_depthTextureUnitLocation  == INVALID_UNIFORM_LOCATION ||
        m_sampleRadLocation         == INVALID_UNIFORM_LOCATION ||
        m_projMatrixLocation        == INVALID_UNIFORM_LOCATION ||		
        m_kernelLocation            == INVALID_UNIFORM_LOCATION ||
        m_aspectRatioLocation       == INVALID_UNIFORM_LOCATION ||
        m_tanHalfFOVLocation        == INVALID_UNIFORM_LOCATION) {
        return false;
    }
   
    Enable();
    
    GLExitIfError;
    
    GenKernel();
    
    glUniform1i(m_depthTextureUnitLocation, DEPTH_TEXTURE_UNIT_INDEX);
    
    GLExitIfError;
    
    return GLCheckError();
}


void SSAOTechnique::GenKernel()
{
    Vector3f kernel[KERNEL_SIZE];
    
    for (uint i = 0 ; i < KERNEL_SIZE ; i++ ) {
        float scale = (float)i / (float)(KERNEL_SIZE);        
        Vector3f v;
        v.x = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        v.y = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        v.z = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        // Use an acceleration function so more points are
        // located closer to the origin
        v *= (0.1f + 0.9f * scale * scale);
        
        kernel[i] = v;
    }
       
    glUniform3fv(m_kernelLocation, KERNEL_SIZE, (const GLfloat*)&kernel[0]);    
}


void SSAOTechnique::BindDepthBuffer(IOBuffer& depthBuf)
{
    depthBuf.BindForReading(DEPTH_TEXTURE_UNIT);
}


void SSAOTechnique::SetSampleRadius(float sr)
{
    glUniform1f(m_sampleRadLocation, sr);
}


void SSAOTechnique::SetProjMatrix(const Matrix4f& m)
{
    glUniformMatrix4fv(m_projMatrixLocation, 1, GL_TRUE, (const GLfloat*)m.m);    
}


void SSAOTechnique::SetAspectRatio(float aspectRatio)
{
    glUniform1f(m_aspectRatioLocation, aspectRatio);
}


void SSAOTechnique::SetTanHalfFOV(float tanHalfFOV)
{
    glUniform1f(m_tanHalfFOVLocation, tanHalfFOV);
}
