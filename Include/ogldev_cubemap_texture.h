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
#define OGLDEV_CUBEMAP_TEXTURE_H

#include <string>
#include <vector>
#include <GL/glew.h>

#include "bitmap.h"

class BaseCubmapTexture {
public:

    virtual void Load() = 0;

    virtual void Bind(GLenum TextureUnit) = 0;

};

class CubemapTexture : public BaseCubmapTexture
{
public:
    CubemapTexture(const std::string& Directory,
                   const std::string& PosXFilename,
                   const std::string& NegXFilename,
                   const std::string& PosYFilename,
                   const std::string& NegYFilename,
                   const std::string& PosZFilename,
                   const std::string& NegZFilename);

    ~CubemapTexture();

    virtual void Load();

    virtual void Bind(GLenum TextureUnit);

private:

    std::string m_fileNames[6];
    GLuint m_textureObj;
};


// Ect - Equirectangular
class CubemapEctTexture : public BaseCubmapTexture
{
public:
   
    CubemapEctTexture(const std::string& Filename);

    ~CubemapEctTexture() {};

    virtual void Load();

    virtual void Bind(GLenum TextureUnit);

private:

    void LoadCubemapData(const std::vector<Bitmap>& Cubemap);

    std::string m_filename;
    GLuint m_textureObj;
};


#endif  /* OGLDEV_CUBEMAP_TEXTURE_H */
