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

#pragma once

#include <string>

#include "gl_skybox_technique.h"
#include "ogldev_cubemap_texture.h"


class SkyBox
{
public:
    SkyBox();

    ~SkyBox();

    /*void Init(const std::string& Directory,
              const std::string& PosXFilename,
              const std::string& NegXFilename,
              const std::string& PosYFilename,
              const std::string& NegYFilename,
              const std::string& PosZFilename,
              const std::string& NegZFilename);*/

    void Init(GLenum TextureUnit, int TextureUnitIndex);

    void Render(BaseCubmapTexture* pSkyboxTex, const Matrix4f& VP);

private:

    SkyboxTechnique m_skyboxTechnique;
    GLenum m_textureUnit = 0;
};

