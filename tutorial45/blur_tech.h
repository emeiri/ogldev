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

#ifndef BLUR_TECH_H
#define	BLUR_TECH_H

#include "technique.h"
#include "ogldev_math_3d.h"
#include "ogldev_io_buffer.h"

class BlurTech : public Technique {
public:

    BlurTech();

    virtual bool Init();

    void BindInputBuffer(IOBuffer& inputBuf);

private:

    GLuint m_inputTextureUnitLocation;
};


#endif