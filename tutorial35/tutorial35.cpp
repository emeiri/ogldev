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

    Tutorial 35 - Deferred Shading - Part 1
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ds_geom_pass_tech.h"
#include "ogldev_glut_backend.h"
#include "ogldev_basic_mesh.h"
#include "gbuffer.h"

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

class Tutorial35 : public ICallbacks, public OgldevApp
{
public:

    Tutorial35() 
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
    }

    ~Tutorial35()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        if (!m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_DSGeomPassTech.Init()) {
            printf("Error initializing DSGeomPassTech\n");
            return false;
        }      

		m_DSGeomPassTech.Enable();
		m_DSGeomPassTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

        if (!m_mesh.LoadMesh("../Content/phoenix_ugv.md2")) {
			return false;
		}

#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif
        
        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        CalcFPS();
        
        m_scale += 0.05f;

        m_pGameCamera->OnRender();

        DSGeometryPass();
		DSLightPass();
                            
        RenderFPS();
        
        glutSwapBuffers();
    }
    

    void DSGeometryPass()
    {
		m_DSGeomPassTech.Enable();

        m_gbuffer.BindForWriting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Pipeline p;
        p.Scale(0.1f, 0.1f, 0.1f);
        p.Rotate(0.0f, m_scale, 0.0f);
        p.WorldPos(-0.8f, -1.0f, 12.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        m_DSGeomPassTech.SetWVP(p.GetWVPTrans());        
		m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());
        m_mesh.Render();       
    }
     
    void DSLightPass()
    {       
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
        m_gbuffer.BindForReading();
        
        GLint HalfWidth = (GLint)(WINDOW_WIDTH / 2.0f);
        GLint HalfHeight = (GLint)(WINDOW_HEIGHT / 2.0f);
        
        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, HalfHeight, HalfWidth, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, HalfHeight, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, 0, WINDOW_WIDTH, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);	
    }


	void KeyboardCB(OGLDEV_KEY OgldevKey)
	{
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
			break;
		default:
			m_pGameCamera->OnKeyboard(OgldevKey);
		}
	}


	virtual void PassiveMouseCB(int x, int y)
	{
		m_pGameCamera->OnMouse(x, y);
	}
    
    
private:
    
	DSGeomPassTech m_DSGeomPassTech;
    Camera* m_pGameCamera;
    float m_scale;
    BasicMesh m_mesh;
    PersProjInfo m_persProjInfo;
    GBuffer m_gbuffer;
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 35")) {
        return 1;
    }
    
    SRANDOM;
 
    Tutorial35* pApp = new Tutorial35();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}