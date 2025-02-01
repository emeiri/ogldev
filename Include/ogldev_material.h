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

#include "ogldev_texture.h"


struct PBRMaterial
{
    float Roughness = 0.0f;
    bool IsMetal = false;
    Vector3f Color = Vector3f(0.0f, 0.0f, 0.0f);
    Texture* pAlbedo = NULL;
    Texture* pRoughness = NULL;
    Texture* pMetallic = NULL;
    Texture* pNormalMap = NULL;
};


class Material {

 public:

     std::string m_name;

    Vector4f AmbientColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f DiffuseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f SpecularColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);

    PBRMaterial PBRmaterial;

    Texture* pDiffuse = NULL; // base color of the material
    Texture* pNormal = NULL;
    Texture* pSpecularExponent = NULL;

    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;

    ~Material()
    {
        if (pDiffuse) {
            delete pDiffuse;
        }

        if (pSpecularExponent) {
            delete pSpecularExponent;
        }
    }
};


#endif
