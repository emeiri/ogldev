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

#ifndef OGLDEV_CUBEMAP_TEXTURE_H
#define	OGLDEV_CUBEMAP_TEXTURE_H

#include <string>
#include <GL/glew.h>
#include <ImageMagick-6/Magick++.h>

using namespace std;

class CubemapTexture
{
public:
    
    CubemapTexture(const string& Directory,
                   const string& PosXFilename,
                   const string& NegXFilename,
                   const string& PosYFilename,
                   const string& NegYFilename,
                   const string& PosZFilename,
                   const string& NegZFilename);

    ~CubemapTexture();
    
    bool Load();

    void Bind(GLenum TextureUnit);

private:
   
    string m_fileNames[6];
    GLuint m_textureObj;
};

#endif	/* OGLDEV_CUBEMAP_TEXTURE_H */

