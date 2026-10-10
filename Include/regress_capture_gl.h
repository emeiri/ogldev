/*

        Copyright 2026 Etay Meiri

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


    OpenGL capture for the regression test mode (see regress_hook.h).

    Include it after the OpenGL headers and call Regress::InitGL() once the
    context is current. In test mode Regress::EndFrame() then reads the back
    buffer of the default framebuffer on the requested frames.
*/

#pragma once

#include "regress_hook.h"

namespace Regress {

inline bool CaptureGL(const char* pPath, int Width, int Height, void* pApiContext)
{
    (void)pApiContext;

    GLint Viewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, Viewport);

    // The size is known only if the window was created through ForceWindowSize
    if ((Width == 0) || (Height == 0)) {
        Width = Viewport[2];
        Height = Viewport[3];
    }

    GLint PrevPackAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &PrevPackAlignment);

#ifdef GL_READ_FRAMEBUFFER
    GLint PrevReadFBO = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &PrevReadFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
#endif

#ifdef GL_PIXEL_PACK_BUFFER
    GLint PrevPackBuffer = 0;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &PrevPackBuffer);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
#endif

    std::vector<unsigned char> Pixels((size_t)Width * Height * 4);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, Pixels.data());

    bool Ok = (glGetError() == GL_NO_ERROR);

    glPixelStorei(GL_PACK_ALIGNMENT, PrevPackAlignment);

#ifdef GL_PIXEL_PACK_BUFFER
    glBindBuffer(GL_PIXEL_PACK_BUFFER, PrevPackBuffer);
#endif

#ifdef GL_READ_FRAMEBUFFER
    glBindFramebuffer(GL_READ_FRAMEBUFFER, PrevReadFBO);
#endif

    if (!Ok) {
        fprintf(stderr, "Regress: glReadPixels failed\n");
        return false;
    }

    // The alpha channel of the back buffer is of no interest
    for (size_t i = 3; i < Pixels.size(); i += 4) {
        Pixels[i] = 255;
    }

    if (!GetState().InfoWritten) {
        WriteInfo(Width, Height);
        GetState().InfoWritten = true;
    }

    // OpenGL rows go bottom to top
    return WritePNG(pPath, Width, Height, Pixels.data(), true);
}


// Call once after the OpenGL context has been made current
inline void InitGL()
{
    if (!Active()) {
        return;
    }

    SetInfo("gl", (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_VERSION));
    SetCapture(CaptureGL);
}

}
