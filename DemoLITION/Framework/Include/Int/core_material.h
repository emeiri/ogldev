/*

        Copyright 2021 Etay Meiri

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

#include "ogldev_math_3d.h"
#ifdef OGLDEV_VULKAN
#include "ogldev_vulkan_texture.h"
#else
#include "ogldev_texture.h"
#endif

#include "model_data.h"


class CoreMaterial {

 public:

    MaterialData* m_pMaterialData = NULL;
    Texture* pTextures[TEX_TYPE_NUM] = { 0 };

    ~CoreMaterial()
    {
        for (Texture* pTex : pTextures) {
            delete pTex;
        }
    }
};
