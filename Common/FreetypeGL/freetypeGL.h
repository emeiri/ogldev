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

#ifndef FREETYPEGL_H
#define FREETYPEGL_H

#include <GL/glew.h>
#include "font-manager.h"
#include "markup.h"
#include "texture-font.h"
#include "text-buffer.h"
#include "mat4.h"
#include "shader.h"

using namespace ftgl;


static vec4 white = { {1.0f,1.0f,1.0f,1.0f} };
static vec4 blue = { {0.0f,0.0f,1.0f,1.0f} };
static vec4 red = { {1.0f,0.0f,0.0f,1.0f} };
static vec4 black = { {0.0f,0.0f,0.0f,1.0f} };
static vec4 yellow = { {1.0f, 1.0f, 0.0f, 1.0f} };
static vec4 orange1 = { {1.0f, 0.9f, 0.0f, 1.0f} };
static vec4 orange2 = { {1.0f, 0.6f, 0.0f, 1.0f} };
static vec4 green = { {0.0f,1.0f,0.0f,1.0f} };
static vec4 gray = { {0.5f,0.5f,0.5f,1.0f} };
static vec4 cyan = { {0.0f,1.0f,1.0f,1.0f} };
static vec4 purple = { {0.5f,0.0f,0.5f,1.0f} };
static vec4 none = { {0.0f,0.0f,1.0f,0.0f} };


enum FONT_TYPE {
    FONT_TYPE_AMIRI,
    FONT_TYPE_LIBERASTIKA,
    FONT_TYPE_LOBSTER,
    FONT_TYPE_LUCKIEST_GUY,
    FONT_TYPE_OLD_STANDARD,
    FONT_TYPE_SOURCE_CODE_PRO,
    FONT_TYPE_SOURCE_SANS_PRO,
    FONT_TYPE_VERA,
    FONT_TYPE_VERA_MOBD,
    FONT_TYPE_VERA_MOBI,
    FONT_TYPE_VERA_MONO,
    NUM_FONTS
};


class FontRenderer
{
public:
    FontRenderer();

    ~FontRenderer();

    void InitFontRenderer(int WindowWidth, int WindowHeight);

    void RenderText(FONT_TYPE FontType, 
                    const vec4& TopColor,
                    const vec4& BottomColor,
                    unsigned int x, 
                    unsigned int y, 
                    const char* pText);

    void RenderText(FONT_TYPE FontType,
                    const vec4& Color,
                    unsigned int x,
                    unsigned int y,
                    const char* pText)
    {
        RenderText(FontType, Color, Color, x, y, pText);
    }

private:

    void LoadFonts();

    texture_atlas_t* m_pAtlas = NULL;
    vertex_buffer_t* m_pBuffer = NULL;
    texture_font_t* m_pFonts[NUM_FONTS] = {};
    GLuint m_shaderProg = -1;
    mat4 m_model, m_view, m_projection;
};


#endif  /* FREETYPEGL_H */
