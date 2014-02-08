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

#ifndef INTERMEDIATE_BUFFER_H
#define	INTERMEDIATE_BUFFER_H

#include <GL/glew.h>


class IntermediateBuffer
{
public:

    IntermediateBuffer();

    ~IntermediateBuffer();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting();

    void BindForReading();  

private:

    GLuint m_fbo;
    GLuint m_colorBuffer;
    GLuint m_motionBuffer;
	GLuint m_depthBuffer;
};

#endif	

