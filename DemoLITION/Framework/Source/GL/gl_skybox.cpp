/*

        Copyright 2025 Etay Meiri

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

#include "GL/gl_skybox.h"

SkyBox::SkyBox()
{
}


SkyBox::~SkyBox()
{
}


/*void SkyBox::Init(const string& Directory,
                  const string& PosXFilename,
                  const string& NegXFilename,
                  const string& PosYFilename,
                  const string& NegYFilename,
                  const string& PosZFilename,
                  const string& NegZFilename)
{
    InitTechnique();

    m_pCubemapTex = new CubemapTexture(Directory,
                                       PosXFilename,
                                       NegXFilename,
                                       PosYFilename,
                                       NegYFilename,
                                       PosZFilename,
                                       NegZFilename);
}*/


void SkyBox::Init(GLenum TextureUnit, int TextureUnitIndex)
{
    m_textureUnit = TextureUnit;

    if (!m_skyboxTechnique.Init()) {
        printf("Error initializing the skybox technique\n");
        exit(1);
    }

    m_skyboxTechnique.Enable();
    m_skyboxTechnique.SetTextureUnitIndex(TextureUnitIndex);

    glGenVertexArrays(1, &m_dummyVAO);
}


void SkyBox::Render(BaseCubmapTexture* pSkyboxTex, const Matrix4f& VP)
{
    if (!pSkyboxTex) {
        printf("Skybox texture was not loaded\n");
        exit(1);
    }

    m_skyboxTechnique.Enable();

    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);

    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_FRONT);

    glDepthFunc(GL_LEQUAL);

    m_skyboxTechnique.SetVP(VP);

    pSkyboxTex->Bind(m_textureUnit);

    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glCullFace(OldCullFaceMode);

    glDepthFunc(OldDepthFuncMode);
}
