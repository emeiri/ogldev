/*
        Copyright 2023 Etay Meiri

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

#include "ogldev_math_3d.h"
#include "ogldev_gui_texture.h"


#define GUI_TEXTURE_UNIT       GL_TEXTURE0
#define GUI_TEXTURE_UNIT_INDEX 0


GUITexture::~GUITexture()
{
}


void GUITexture::Init(int x, int y, int Width, int Height)
{
    m_screenQuad.Init(x, y, Width, Height);
    m_guiTexTech.Init();
    m_guiTexTech.Enable();
    m_guiTexTech.SetTextureUnit(GUI_TEXTURE_UNIT_INDEX);
}


void GUITexture::Render(GLuint Texture)
{
    glActiveTexture(GUI_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, Texture);

    m_guiTexTech.Enable();

    m_screenQuad.Render();
}
