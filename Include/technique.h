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

#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <list>
#include <GL/glew.h>

class Technique
{
public:

    Technique();

    virtual ~Technique();

    virtual bool Init();

    void Enable();

    GLuint GetProgram() const { return m_shaderProg; }

protected:

    bool AddShader(GLenum ShaderType, const char* pFilename);

    bool Finalize();

    GLint GetUniformLocation(const char* pUniformName);

    GLuint m_shaderProg = 0;

private:

    void PrintUniformList();

    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};

#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         \
    if (loc == INVALID_UNIFORM_LOCATION)    \
        return false;                       
#else
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         
#endif


#endif  /* TECHNIQUE_H */
