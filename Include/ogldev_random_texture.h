/*

	Copyright 2015 Etay Meiri

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

#ifndef OGLDEV_RANDOM_TEXTURE_H
#define OGLDEV_RANDOM_TEXTURE_H

#include <GL/glew.h>

#include "ogldev_types.h"

class RandomTexture
{
public:
    RandomTexture();
    
    bool Init(uint size);
    
    ~RandomTexture();
    
    void Bind(GLenum TextureUnit);
    
private:
    uint m_texture;
};

#endif
