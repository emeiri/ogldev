/*

	Copyright 2013 Etay Meiri

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

    Tutorial 45 - Screen Space Ambient Occlusion
*/

#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "lighting_technique.h"
#include "geom_pass_tech.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "mesh.h"
#include "gbuffer.h"

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

class Tutorial45 : public ICallbacks, public OgldevApp
{
public:

    Tutorial45() 
    {
        m_pGameCamera = NULL;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 1000.0f;  
        
        m_pipeline.SetPerspectiveProj(m_persProjInfo);           
        //m_pipeline.WorldPos(Vector3f(0.0f, 0.0f, 0.0f));        
        //m_pipeline.Scale(0.1f, 0.1f, 0.1f);                		
    }

    ~Tutorial45()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 1.0f, -5.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
              
        if (!m_LightingTech.Init()) {
            OGLDEV_ERROR("Error initializing the lighting technique\n");
            return false;
        }

        m_LightingTech.Enable();

        m_LightingTech.SetPositionTextureUnit(GBUFFER_POSITION_TEXTURE_UNIT);
        m_LightingTech.SetNormalTextureUnit(GBUFFER_NORMAL_TEXTURE_UNIT);
        m_LightingTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);        

        if (!m_mesh.LoadMesh("../Content/dragon.obj")) {
            return false;            
        }        
        m_mesh.GetOrientation().m_pos = Vector3f(0.0f, 0.0f, 5.0f);
        
        if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }
        
        if (!m_gBuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }
                       
#ifndef WIN32
        // Disabled for now because it somehow clashes with the regular rendering...
 //       if (!m_fontRenderer.InitFontRenderer()) {
   //         return false;
   //     }
#endif        	      
        return true;
    }

    void Run()
    {
        OgldevBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        m_pGameCamera->OnRender();      

        m_pipeline.SetCamera(*m_pGameCamera);
		
        GeometryPass();
        
        RenderPass();
		      	
    //    RenderFPS();     
        CalcFPS();
        
        OgldevBackendSwapBuffers();
    }
    
    void GeometryPass()
    {
		m_geomPassTech.Enable();

        m_gBuffer.BindForWriting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pipeline.Orient(m_mesh.GetOrientation());
        m_geomPassTech.SetWVP(m_pipeline.GetWVPTrans());        
		m_geomPassTech.SetWorldMatrix(m_pipeline.GetWorldTrans());
        m_mesh.Render();       
    }
    
    
    void RenderPass()
    {
        m_LightingTech.Enable();
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);        
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_gBuffer.BindForReading();
                           
        m_quad.Render();                
    }
	
       
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey)
    {
        switch (OgldevKey) {
            case OGLDEV_KEY_ESCAPE:
            case OGLDEV_KEY_q:
                OgldevBackendLeaveMainLoop();
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
        
    LightingTechnique m_LightingTech;
    GeomPassTech m_geomPassTech;
    Camera* m_pGameCamera;
    Mesh m_mesh;
    Mesh m_quad;
    PersProjInfo m_persProjInfo;
    Pipeline m_pipeline;
    GBuffer m_gBuffer;
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);

    OgldevBackendInit(OGLDEV_BACKEND_TYPE_GLFW, argc, argv, true, false);

    if (!OgldevBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 45")) {
        OgldevBackendTerminate();
		return 1;
    }

    SRANDOM;
    
    Tutorial45* pApp = new Tutorial45();

    if (!pApp->Init()) {
		delete pApp;
		OgldevBackendTerminate();
        return 1;
    }

    pApp->Run();

    delete pApp;

	OgldevBackendTerminate();
 
    return 0;
}