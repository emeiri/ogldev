/* =========================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * -------------------------------------------------------------------------
 * Copyright 2011 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ========================================================================= */
#include <GL/glew.h>
#if defined(__APPLE__)
    #include <Glut/glut.h>
#else
    #include <GL/glut.h>
#endif

#include <stdio.h>
#include <string.h>

#include "font_shader.h"
#include "font-manager.h"

static const char* pVS = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 Position;                                       \n\
layout (location = 1) in vec2 TexCoord;                                       \n\
layout (location = 2) in vec4 Color;                                          \n\
                                                                              \n\
out vec2 TexCoordOut;                                                         \n\
out vec4 ColorOut;                                                            \n\
                                                                              \n\
uniform mat4 gTrans;                                                          \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    gl_Position = gTrans * vec4(Position, 1.0);                               \n\
    TexCoordOut = TexCoord;                                                   \n\
    ColorOut = Color;                                                         \n\
}";

static const char* pFS = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
out vec4 FragColor;                                                           \n\
                                                                              \n\
in vec2 TexCoordOut;                                                          \n\
in vec4 ColorOut;                                                             \n\
                                                                              \n\
uniform sampler2D gFontMap;                                                   \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    vec4 Color = texture2D(gFontMap, TexCoordOut);                            \n\
    FragColor = vec4(Color.w) * ColorOut;                                     \n\
}";


FontShader::FontShader()
{
    m_shaderProg = 0;    
    m_transLoc = 0;
}


FontShader::~FontShader()
{
    if (m_shaderProg) {
        glDeleteProgram(m_shaderProg);
    }
}

GLuint FontShader::AddShader(const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        return 0;
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0]= strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        return 0;
    }
    
    glAttachShader(m_shaderProg, ShaderObj);
    
    return ShaderObj;
}


bool FontShader::InitFontShader()
{
    //
    // Stage 1 - compile the shaders and link the program
    //
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
    GLuint vs, fs;

    m_shaderProg = glCreateProgram();

    if (m_shaderProg == 0) {
        fprintf(stderr, "Error creating shader program\n");
        goto exit_error;
    }

    vs = AddShader(pVS, GL_VERTEX_SHADER);
    fs = AddShader(pFS, GL_FRAGMENT_SHADER);

    glLinkProgram(m_shaderProg);
    glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        goto exit_error;
    }

    glValidateProgram(m_shaderProg);
    glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        goto exit_error;
    }

    //
    // Stage 2 - get the locations of the uniforms
    //
    m_transLoc = glGetUniformLocation(m_shaderProg, "gTrans");

    if (m_transLoc == 0xffffffff) {
        fprintf(stderr, "%s:%d - error getting uniform location of 'gTrans'\n", __FILE__, __LINE__);
        goto exit_error;
    }
    
    m_fontMapLoc = glGetUniformLocation(m_shaderProg, "gFontMap");
   
    if (m_fontMapLoc == 0xffffffff) {
        fprintf(stderr, "%s:%d - error getting the uniform location of 'gFontMap'\n", __FILE__, __LINE__);
        return false;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return true;
    
exit_error:
        
    if (m_shaderProg) {
        glDeleteProgram(m_shaderProg);        
    }

    if (vs) {
        glDeleteShader(vs);
    }

    if (fs) {
        glDeleteShader(fs);
    }

    return false;        
}


void FontShader::SetTransformation(const GLfloat* pTrans)
{
    glUniformMatrix4fv(m_transLoc, 1, GL_TRUE, pTrans);
}


void FontShader::SetFontMapTextureUnit(unsigned int TextureUnitIndex)
{
    glUniform1i(m_fontMapLoc, TextureUnitIndex);
}


void FontShader::Enable()
{
    glUseProgram(m_shaderProg);
}