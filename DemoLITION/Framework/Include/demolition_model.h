/*

        Copyright 2024 Etay Meiri

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

#include "demolition_object.h"

class Model : public Object 
{
public:

    virtual void SetColorTexture(int TextureHandle) = 0;

    virtual void SetNormalMap(int TextureHandle) = 0;

    virtual void SetHeightMap(int TextureHandle) = 0;

    virtual void SetTextureScale(float Scale) = 0;
};


class Grid : public Model
{
};

