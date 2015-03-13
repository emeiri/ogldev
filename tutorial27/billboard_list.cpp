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

#include "ogldev_util.h"
#include "ogldev_engine_common.h"
#include "billboard_list.h"

#define NUM_ROWS 10
#define NUM_COLUMNS 10


BillboardList::BillboardList()
{
    m_pTexture = NULL;
    m_VB = INVALID_OGL_VALUE;
}


BillboardList::~BillboardList()
{
    SAFE_DELETE(m_pTexture);
    
    if (m_VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &m_VB);
    }
}
    
    
bool BillboardList::Init(const std::string& TexFilename)
{
    m_pTexture = new Texture(GL_TEXTURE_2D, TexFilename.c_str());
        
    if (!m_pTexture->Load()) {
        return false;
    }

    CreatePositionBuffer();
    
    if (!m_technique.Init()) {
        return false;
    }
    
    return true;
}


void BillboardList::CreatePositionBuffer()
{    
    Vector3f Positions[NUM_ROWS * NUM_COLUMNS];
    
    for (unsigned int j = 0 ; j < NUM_ROWS ; j++) {
        for (unsigned int i = 0 ; i < NUM_COLUMNS ; i++) {
            Vector3f Pos((float)i, 0.0f, (float)j);            
            Positions[j * NUM_COLUMNS + i] = Pos;
        }
    }

    glGenBuffers(1, &m_VB);
  	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions), &Positions[0], GL_STATIC_DRAW);
}


void BillboardList::Render(const Matrix4f& VP, const Vector3f& CameraPos)
{
    m_technique.Enable();
    m_technique.SetVP(VP);
    m_technique.SetCameraPosition(CameraPos);
    
    m_pTexture->Bind(COLOR_TEXTURE_UNIT);
    
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);   // position
    
    glDrawArrays(GL_POINTS, 0, NUM_ROWS * NUM_COLUMNS);
    
    glDisableVertexAttribArray(0);
}