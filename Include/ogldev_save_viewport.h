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

#pragma once

#include <GL/glew.h>

class SaveViewport {
public:
    SaveViewport() {}

    void Save()
    {
        glGetIntegerv(GL_VIEWPORT, m_viewport);
    }

    void Restore()
    {
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
    }

private:
    GLint m_viewport[4];
};
