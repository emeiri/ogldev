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

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include <GL/glew.h>

// In the youtube series I've switch to stb_image
//#define USE_IMAGE_MAGICK

#ifdef USE_IMAGE_MAGICK
#include <ImageMagick-6/Magick++.h>
#endif

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    // Should be called once to load the texture
    bool Load();

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
#ifdef USE_IMAGE_MAGICK
    Magick::Image m_image;
    Magick::Blob m_blob;
#endif
};


#endif  /* TEXTURE_H */
