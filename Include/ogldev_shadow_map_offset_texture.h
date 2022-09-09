/*

        Copyright 2022 Etay Meiri

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

#ifndef SHADOW_MAP_OFFSET_TEXTURE_H
#define SHADOW_MAP_OFFSET_TEXTURE_H

#include <GL/glew.h>

class ShadowMapOffsetTexture {

 public:
    ShadowMapOffsetTexture(int WindowSize, int FilterSize);

    void Bind(GLenum TextureUnit);

 private:
    GLuint m_textureObj;

    void CreateTexture(int WindowSize, int FilterSize, const vector<float>& Data);
};

#endif
