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

#include "skybox.h"
#include "pipeline.h"
#include "ogldev_util.h"

SkyBox::SkyBox(const Camera* pCamera,
               const PersProjInfo& p)
{
    m_pCamera = pCamera;
    m_persProjInfo = p;            
    
    m_pSkyboxTechnique = NULL;
    m_pCubemapTex = NULL;
    m_pMesh = NULL;
}


SkyBox::~SkyBox()
{
    SAFE_DELETE(m_pSkyboxTechnique);
    SAFE_DELETE(m_pCubemapTex);
    SAFE_DELETE(m_pMesh);
}


bool SkyBox::Init(const string& Directory,
                  const string& PosXFilename,
                  const string& NegXFilename,
                  const string& PosYFilename,
                  const string& NegYFilename,
                  const string& PosZFilename,
                  const string& NegZFilename)
{
    m_pSkyboxTechnique = new SkyboxTechnique();

    if (!m_pSkyboxTechnique->Init()) {
        printf("Error initializing the skybox technique\n");
        return false;
    }

    m_pSkyboxTechnique->Enable();
    m_pSkyboxTechnique->SetTextureUnit(0);
    
    m_pCubemapTex = new CubemapTexture(Directory,
                                       PosXFilename,
                                       NegXFilename,
                                       PosYFilename,
                                       NegYFilename,
                                       PosZFilename,
                                       NegZFilename);

    if (!m_pCubemapTex->Load()) {
        return false;
    }
        
    m_pMesh = new Mesh();

    return m_pMesh->LoadMesh("../Content/sphere.obj"); 
}


void SkyBox::Render()
{
    m_pSkyboxTechnique->Enable();
    
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);

    Pipeline p;    
    p.Scale(20.0f, 20.0f, 20.0f);
    p.Rotate(0.0f, 0.0f, 0.0f);
    p.WorldPos(m_pCamera->GetPos().x, m_pCamera->GetPos().y, m_pCamera->GetPos().z);
    p.SetCamera(m_pCamera->GetPos(), m_pCamera->GetTarget(), m_pCamera->GetUp());
    p.SetPerspectiveProj(m_persProjInfo);
    m_pSkyboxTechnique->SetWVP(p.GetWVPTrans());
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();  
    
    glCullFace(OldCullFaceMode);        
    glDepthFunc(OldDepthFuncMode);
}