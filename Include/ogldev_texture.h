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

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    Texture(GLenum TextureTarget);

    // Should be called once to load the texture
    bool Load();

    void Load(unsigned int BufferSize, void* pImageData);

    void Load(const std::string& Filename);

    void LoadRaw(int Width, int Height, int BPP, const unsigned char* pImageData);

    void LoadF32(int Width, int Height, const float* pImageData);

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

    GLuint64 GetBindlessHandle() const { return m_bindlessHandle; }

private:
    void LoadInternal(const void* pImageData);
    void LoadInternalNonDSA(const void* pImageData);
    void LoadInternalDSA(const void* pImageData);    

    void BindInternalNonDSA(GLenum TextureUnit);
    void BindInternalDSA(GLenum TextureUnit);

    std::string m_fileName;
    GLenum m_textureTarget = 0;
    GLuint m_textureObj = 0;
    GLuint64 m_bindlessHandle = 0;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};


#endif  /* TEXTURE_H */
