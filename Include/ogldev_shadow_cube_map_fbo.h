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

#ifndef SHADOW_CUBE_MAP_FBO_H
#define SHADOW_CUBE_MAP_FBO_H

#include <GL/glew.h>

#include "ogldev_types.h"

class ShadowCubeMapFBO
{
public:

    ShadowCubeMapFBO();

    ~ShadowCubeMapFBO();

    bool Init(uint size);

    void BindForWriting(GLenum CubeFace);

    void BindForReading(GLenum TextureUnit);

private:

    uint m_size = 0;
    GLuint m_fbo;
    GLuint m_shadowCubeMap;
    GLuint m_depth;
};

#endif  /* SHADOW_CUBE_MAP_FBO_H */
