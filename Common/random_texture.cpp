/*

	Copyright 2014 Etay Meiri

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

#include "ogldev_random_texture.h"
#include "ogldev_util.h"


RandomTexture::RandomTexture()
{
    m_texture = 0;
}


RandomTexture::~RandomTexture()
{
    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
    }
}


bool RandomTexture::Init()
{
    glGenTextures(1, &m_texture);    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return GLCheckError();
}

void RandomTexture::Bind(GLenum TextureUnit)
{
        
}