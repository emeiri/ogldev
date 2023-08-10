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

#ifndef BILLBOARD_LIST_H
#define	BILLBOARD_LIST_H

#include <string>

#include "ogldev_texture.h"
#include "billboard_technique.h"

class BillboardList
{
public:
    BillboardList();    
    ~BillboardList();
    
    bool Init(const std::string& TexFilename);
    
    void Render(const Matrix4f& VP, const Vector3f& CameraPos);

private:
    void CreatePositionBuffer();
    
    GLuint m_VB;
    Texture* m_pTexture;
    BillboardTechnique m_technique;
};


#endif	/* BILLBOARD_LIST_H */

