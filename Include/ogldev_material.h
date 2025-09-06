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

#ifndef OGLDEV_MATERIAL_H
#define OGLDEV_MATERIAL_H

#include "ogldev_math_3d.h"
#ifdef OGLDEV_VULKAN
#include "ogldev_vulkan_texture.h"
#else
#include "ogldev_texture.h"
#endif

struct PBRMaterial
{
    float Roughness = 0.0f;
    bool IsMetal = false;
    Vector3f Color = Vector3f(0.0f, 0.0f, 0.0f);
    Texture* pAlbedo = NULL;
    Texture* pRoughness = NULL;
    Texture* pMetallic = NULL;
    Texture* pNormalMap = NULL;
    Texture* pAO = NULL;
    Texture* pEmissive = NULL;
};

enum TEXTURE_TYPE {
    TEX_TYPE_BASE = 0,      // Base color / diffuse / albedo
    TEX_TYPE_SPECULAR = 1,
    TEX_TYPE_NORMAL = 2,
    TEX_TYPE_METALNESS = 3,
    TEX_TYPE_EMISSIVE = 4,
    TEX_TYPE_NORMAL_CAMERA = 5,
    TEX_TYPE_EMISSION_COLOR = 6,
    TEX_TYPE_ROUGHNESS = 7,
    TEX_TYPE_AMBIENT_OCCLUSION = 8,
    TEX_TYPE_NUM = 9
};

class Material {

 public:

    std::string m_name;

    Vector4f AmbientColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f DiffuseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f SpecularColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f BaseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f EmissiveColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f MetallicRoughnessNormalOcclusion = Vector4f(1.0f);

    PBRMaterial PBRmaterial;

    Texture* pTextures[TEX_TYPE_NUM] = { 0 };

    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;

    ~Material()
    {
        for (Texture* pTex : pTextures) {
            delete pTex;
        }
    }
};


#endif
