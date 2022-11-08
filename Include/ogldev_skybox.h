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
#include "ogldev_basic_mesh.h"

class SkyBox
{
public:
    SkyBox(const BasicCamera* pCamera);

    ~SkyBox();

    bool Init(const string& Directory,
              const string& PosXFilename,
              const string& NegXFilename,
              const string& PosYFilename,
              const string& NegYFilename,
              const string& PosZFilename,
              const string& NegZFilename);

    void Render();

private:
    SkyboxTechnique* m_pSkyboxTechnique;
    const BasicCamera* m_pCamera;
    CubemapTexture* m_pCubemapTex;
    BasicMesh* m_pMesh;
};

#endif  /* OGLDEV_SKYBOX_H */
