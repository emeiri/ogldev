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

#include <stdlib.h>
#include <GL/glew.h>
#include <assert.h>

#include "freetypeGL.h"

extern "C" {
#include "vector.h"
#include "markup.h"
#include "texture-font.h"
#include "texture-glyph.h"
#include "vertex-buffer.h"
#include "font_shader.h"
}

#define MAX_STRING_LEN 128

Markup sDefaultMarkup = { (char*)"Arial", 64, 0, 0, 0.0, 0.0,
                          {.5,.5,.5,.5}, {1,1,1,0},
                          0, {0,0,0,1}, 0, {0,0,0,1},
                          0, {0,0,0,1}, 0, {0,0,0,1} };


FontRenderer::FontRenderer()
{
    m_pManager = NULL;    
    m_pFont = NULL;    
    m_pTextBuffer = NULL;    
    m_markup = sDefaultMarkup;
}

    
FontRenderer::FontRenderer(const Markup& markup)
{
    m_pManager = NULL;    
    m_pFont = NULL;    
    m_pTextBuffer = NULL;        
    m_markup = markup;
}

FontRenderer::~FontRenderer()
{
    if (m_pManager) {
        font_manager_delete(m_pManager);
    }
    
    if (m_pTextBuffer) {
        vertex_buffer_delete(m_pTextBuffer);
    }
}


bool FontRenderer::InitFontRenderer()
{
    m_pManager = font_manager_new( 512, 512, 1 );
    
    if (!m_pManager) {
        fprintf(stderr, "%s:%d - error initializing the font manager\n", __FILE__, __LINE__);
        return false;
    }
    
    m_pFont = font_manager_get_from_markup( m_pManager, &m_markup );

    if (!m_pFont) {
        fprintf(stderr, "%s:%d - error getting the font\n", __FILE__, __LINE__);
        return false;
    }

    m_pTextBuffer = vertex_buffer_new( "v3f:t2f:c4f" );
    
    if (!m_pTextBuffer) {
        fprintf(stderr, "%s:%d - error creating the text buffer\n", __FILE__, __LINE__);
        return false;
    }
       
    if (!m_fontShader.InitFontShader()) {
        fprintf(stderr, "%s:%d - error compiling the shader program\n", __FILE__, __LINE__);
        return false;
    }    
    
    m_fontShader.Enable();
    m_fontShader.SetFontMapTextureUnit(0);
    
    return true;    
}


void FontRenderer::RenderText(unsigned int x, unsigned int y, const char* pText)
{
    bool IsCullEnabled = glIsEnabled(GL_CULL_FACE);
    
    glDisable(GL_CULL_FACE);
    
    assert(strlen(pText) < MAX_STRING_LEN);
        
    wchar_t text[MAX_STRING_LEN] = { 0 };
    
    int len = mbstowcs(text, pText, strlen(pText));
    assert(len > 0);

    vertex_buffer_clear( m_pTextBuffer );
    
    TextureGlyph* pGlyph = texture_font_get_glyph( m_pFont, text[0] );
    
    assert(pGlyph);

    Pen pen = {(float)x, (float)y};
    texture_glyph_add_to_vertex_buffer( pGlyph, m_pTextBuffer, &m_markup, &pen, 0 );
    
    for( size_t i=1; i<wcslen(text); ++i )
    {
        pGlyph = texture_font_get_glyph( m_pFont, text[i] );
        assert(pGlyph);
        
        int kerning = texture_glyph_get_kerning( pGlyph, text[i-1] );
        texture_glyph_add_to_vertex_buffer( pGlyph, m_pTextBuffer, &m_markup, &pen, kerning );
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, m_pManager->atlas->texid );

    m_fontShader.Enable();
    
    int viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

    float xScale = 2.0f / (float)viewport[2];
    float yScale = 2.0f / (float)viewport[3];
    
    GLfloat Trans[] = { xScale, 0.0f,   0.0f, -1.0f,
                        0.0f,   yScale, 0.0f, -1.0f,
                        0.0f,   0.0f,   1.0f,  0.0f,
                        0.0f,   0.0f,   0.0f,  1.0f };

    m_fontShader.SetTransformation(Trans);
    
    vertex_buffer_render( m_pTextBuffer, GL_TRIANGLES, "vtc" );
    
    if (IsCullEnabled) {
        glEnable(GL_CULL_FACE);
    }
}

