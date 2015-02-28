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


bool RandomTexture::Init(uint size)
{
    glGenTextures(1, &m_texture);    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    uint TextureSize = size * size * 3;
    float* pRandom = new float[TextureSize];
    for (uint i = 0 ; i < TextureSize ; i++) {
        float r = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        pRandom[i] = r;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, pRandom);
    
    delete [] pRandom;
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return GLCheckError();
}

void RandomTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_texture);        
}