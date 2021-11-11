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

#ifndef LIGHTING_TECHNIQUE_17_H
#define LIGHTING_TECHNIQUE_17_H

#include "technique.h"
#include "ogldev_math_3d.h"
#include "ogldev_material.h"

class BaseLight
{
public:
    Vector3f Color;
    float AmbientIntensity;

    BaseLight()
    {
        Color = Vector3f(1.0f, 1.0f, 1.0f);
        AmbientIntensity = 0.0f;
    }
};



class LightingTechnique : public Technique
{
public:

    LightingTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetLight(const BaseLight& Light);
    void SetMaterial(const Material& material);

private:

    GLuint WVPLoc;
    GLuint samplerLoc;
    GLuint lightColorLoc;
    GLuint lightAmbientIntensityLoc;
    GLuint materialAmbientColorLoc;
};


#endif  /* LIGHTING_TECHNIQUE_H */
