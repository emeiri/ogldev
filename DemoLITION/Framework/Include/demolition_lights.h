/*

        Copyright 2023 Etay Meiri

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

class BaseLight
{
public:
    Vector3f Color = Vector3f(1.0f, 1.0f, 1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;

    bool IsZero() const
    {
        return ((AmbientIntensity == 0) && (DiffuseIntensity == 0.0f));
    }
};


class DirectionalLight : public BaseLight
{
public:
    Vector3f WorldDirection = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f Up = Vector3f(0.0f, 1.0f, 0.0f);
};


struct LightAttenuation
{
    float Constant = 0.1f;
    float Linear = 0.0f;
    float Exp = 0.0f;
};


class PointLight : public BaseLight
{
public:
    Vector3f WorldPosition = Vector3f(0.0f, 0.0f, 0.0f);
    LightAttenuation Attenuation;
};


class SpotLight : public PointLight
{
public:
    Vector3f WorldDirection = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f Up = Vector3f(0.0f, 1.0f, 0.0f);
    float Cutoff = 0.0f;
};


struct PBRLight {
    Vector4f PosDir;   // if w == 1 position, else direction
    Vector3f Intensity;
};
