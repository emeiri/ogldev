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

#pragma once

#include "technique.h"
#include "ogldev_math_3d.h"
#include "Int/core_material.h"
#include "ogldev_world_transform.h"
#include "demolition_lights.h"
#include "Int/core_model.h"
#include "GL/gl_base_lighting_technique.h"

class PBRForwardLightingTechnique : public BaseLightingTechnique
{
public:

    PBRForwardLightingTechnique();

    virtual bool Init();

    virtual void SetMaterial(const CoreMaterial& material);

    void SetAmbientOcclusionTextureUnit(int TextureUnit);

    void SetEmissiveTextureUnit(int TextureUnit);

    void SetAlbedoTextureUnit(int TextureUnit);

    void SetRoughnessTextureUnit(int TextureUnit);    

    void SetNormalTextureUnit(int TextureUnit);

    void SetEnvmapTextureUnit(int TextureUnit);

    void SetBRDF_LUTTextureUnit(int TextureUnit);

    void SetIrradianceTextureUnit(int TextureUnit);

    void SetClearCoatTextureUnit(int TextureUnit);
    void SetClearCoatRoughnessTextureUnit(int TextureUnit);
    void SetClearCoatNormalTextureUnit(int TextureUnit);


private:
    DEF_LOC(m_aoUnitLoc);
    DEF_LOC(m_emissiveUnitLoc);
    DEF_LOC(m_albedoUnitLoc);
    DEF_LOC(m_roughnessUnitLoc);
    DEF_LOC(m_normalUnitLoc);
    DEF_LOC(m_envmapUnitLoc);
    DEF_LOC(m_brdfLUTUnitLoc);
    DEF_LOC(m_irradianceUnitLoc);
    DEF_LOC(m_baseColorLoc);
    DEF_LOC(m_emissiveColorLoc);
    DEF_LOC(m_metallicRoughnessNormalOcclusionLoc);
    DEF_LOC(m_clearCoatLoc);
    DEF_LOC(m_clearCoatRoughnessLoc);
    DEF_LOC(m_clearCoatNormalLoc);
    DEF_LOC(m_clearCoatTransmissionThicknessLoc);
};

