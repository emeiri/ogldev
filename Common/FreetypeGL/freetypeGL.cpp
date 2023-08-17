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

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <assert.h>

#include "freetypeGL.h"
#include "markup.h"
#include "vertex-buffer.h"
#include "text-buffer.h"

extern "C" {
#include "mat4.c"
#include "shader.c"
}

using namespace ftgl;

#define MAX_STRING_LEN 128

vec4 white = { {1.0f,1.0f,1.0f,1.0f} };
vec4 black = { {0.0f,0.0f,0.0f,1.0f} };
vec4 yellow = { {1.0f, 1.0f, 0.0f, 1.0f} };
vec4 orange1 = { {1.0f, 0.9f, 0.0f, 1.0f} };
vec4 orange2 = { {1.0f, 0.6f, 0.0f, 1.0f} };
vec4 none = { {0.0f,0.0f,1.0f,0.0f} };

typedef struct {
    float x, y, z;
    float u, v;
    vec4 color;
} vertex_t;


void add_text(vertex_buffer_t* m_pBuffer, texture_font_t* m_pFont, char* text, vec2 pen, vec4 fg_color_1, vec4 fg_color_2)
{
    for (size_t i = 0; i < strlen(text); ++i)
    {
        texture_glyph_t* glyph = texture_font_get_glyph(m_pFont, text + i);
        float kerning = 0.0f;
        if (i > 0)
        {
            kerning = texture_glyph_get_kerning(glyph, text + i - 1);
        }
        pen.x += kerning;

        /* Actual glyph */
        float x0 = (pen.x + glyph->offset_x);
        float y0 = (float)((int)(pen.y + glyph->offset_y));
        float x1 = (x0 + glyph->width);
        float y1 = (float)((int)(y0 - glyph->height));
        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;
        GLuint index = (GLuint)m_pBuffer->vertices->size;
        GLuint indices[] = { index, index + 1, index + 2,
                            index, index + 2, index + 3 };
        vertex_t vertices[] = {
            { (float)((int)x0),y0,0,  s0,t0,  fg_color_1 },
            { (float)((int)x0),y1,0,  s0,t1,  fg_color_2 },
            { (float)((int)x1),y1,0,  s1,t1,  fg_color_2 },
            { (float)((int)x1),y0,0,  s1,t0,  fg_color_1 } };
        vertex_buffer_push_back_indices(m_pBuffer, indices, 6);
        vertex_buffer_push_back_vertices(m_pBuffer, vertices, 4);
        pen.x += glyph->advance_x;
    }
}


FontRenderer::FontRenderer()
{
}


FontRenderer::~FontRenderer()
{
}


void FontRenderer::InitFontRenderer(int WindowWidth, int WindowHeight)
{
    m_pAtlas = texture_atlas_new(1024, 1024, 1);
    m_pBuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
    m_pFont = texture_font_new_from_file(m_pAtlas, 128, "../Common/FreetypeGL/fonts/LuckiestGuy.ttf");

    glGenTextures(1, &m_pAtlas->id);
    glBindTexture(GL_TEXTURE_2D, m_pAtlas->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)m_pAtlas->width, (GLsizei)m_pAtlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pAtlas->data);

    m_shaderProg = shader_load("../Common/FreetypeGL/v3f-t2f-c4f.vert", "../Common/FreetypeGL/v3f-t2f-c4f.frag");

    mat4_set_identity(&m_model);
    mat4_set_identity(&m_view);
    mat4_set_orthographic(&m_projection, 0, (float)WindowWidth, 0, (float)WindowHeight, -1, 1);
}



void FontRenderer::RenderText(unsigned int x, unsigned int y, const char* pText)
{
    vec2 pen = { {(float)x, (float)y} };

    m_pFont->rendermode = RENDER_NORMAL;
    m_pFont->outline_thickness = 0;
    vertex_buffer_clear(m_pBuffer);
    add_text(m_pBuffer, m_pFont, (char*)pText, pen, orange1, orange2);    

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_pAtlas->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)m_pAtlas->width, (GLsizei)m_pAtlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pAtlas->data);

    glUseProgram(m_shaderProg);

    glUniform1i(glGetUniformLocation(m_shaderProg, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProg, "model"), 1, 0, m_model.data);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProg, "view"), 1, 0, m_view.data);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProg, "projection"), 1, 0, m_projection.data);
    vertex_buffer_render(m_pBuffer, GL_TRIANGLES);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
