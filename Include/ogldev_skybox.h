/*

  Copyright 2022 Etay Meiri

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

#ifndef OGLDEV_SKYBOX_H
#define OGLDEV_SKYBOX_H

#include "ogldev_basic_glfw_camera.h"
#include "ogldev_skybox_technique.h"
#include "ogldev_cubemap_texture.h"

class BasicMesh;

class SkyBox
{
public:
    SkyBox();

    ~SkyBox();

    void Init(const std::string& Directory,
              const std::string& PosXFilename,
              const std::string& NegXFilename,
              const std::string& PosYFilename,
              const std::string& NegYFilename,
              const std::string& PosZFilename,
              const std::string& NegZFilename);

    void Init(const std::string& EctTextureFilename);

    void Render(const BasicCamera& pCamera);

    void Render(const Matrix4f& VP);

private:
    void InitTechnique();
    void LoadTextureAndMesh();

    SkyboxTechnique* m_pSkyboxTechnique = NULL;
    BaseCubmapTexture* m_pCubemapTex = NULL;
    BasicMesh* m_pMesh = NULL;
};

#endif  /* OGLDEV_SKYBOX_H */
