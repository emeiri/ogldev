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

    Tutorial 46 - Screen Space Ambient Occlusion With Depth Reconstruction
*/

#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ssao_technique.h"
#include "geom_pass_tech.h"
#include "blur_tech.h"
#include "lighting_technique.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "mesh.h"
#include "ogldev_io_buffer.h"

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

class Tutorial46 : public ICallbacks, public OgldevApp
{
public:

    Tutorial46() 
    {
        m_pGameCamera = NULL;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 5000.0f;  
        
        m_pipeline.SetPerspectiveProj(m_persProjInfo);           
        
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.3f;
        m_directionalLight.DiffuseIntensity = 1.0f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);        
        
        m_shaderType = 0;
    }

    ~Tutorial46()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
   //    Vector3f Pos(0.0f, 23.0f, -5.0f);
  //      Vector3f Target(-1.0f, 0.0f, 0.1f);
        Vector3f Pos(0.0f, 24.0f, -38.0f);
        Vector3f Target(0.0f, -0.5f, 1.0f);

        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);

        if (!m_geomPassTech.Init()) {
            OGLDEV_ERROR0("Error initializing the geometry pass technique\n");
            return false;
        }

        if (!m_SSAOTech.Init()) {
            OGLDEV_ERROR0("Error initializing the SSAO technique\n");
            return false;
        }

        m_SSAOTech.Enable();
        m_SSAOTech.SetSampleRadius(1.5f);
        Matrix4f PersProjTrans;
        PersProjTrans.InitPersProjTransform(m_persProjInfo);
        m_SSAOTech.SetProjMatrix(PersProjTrans);
        float AspectRatio = m_persProjInfo.Width / m_persProjInfo.Height;
        m_SSAOTech.SetAspectRatio(AspectRatio);
        float TanHalfFOV = tanf(ToRadian(m_persProjInfo.FOV / 2.0f));
        m_SSAOTech.SetTanHalfFOV(TanHalfFOV);
        
        if (!m_lightingTech.Init()) {
            OGLDEV_ERROR0("Error initializing the lighting technique\n");
            return false;
        }
        
        m_lightingTech.Enable();
        m_lightingTech.SetDirectionalLight(m_directionalLight);
        m_lightingTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        m_lightingTech.SetShaderType(0);
        
        if (!m_blurTech.Init()) {
            OGLDEV_ERROR0("Error initializing the blur technique\n");
            return false;
        }                
        
        //if (!m_mesh.LoadMesh("../Content/crytek_sponza/sponza.obj")) {
        if (!m_mesh.LoadMesh("../Content/jeep.obj")) {
            return false;            
        }        
     
        m_mesh.GetOrientation().m_scale = Vector3f(0.05f);
        m_mesh.GetOrientation().m_pos = Vector3f(0.0f, 0.0f, 0.0f);
        m_mesh.GetOrientation().m_rotation = Vector3f(0.0f, 180.0f, 0.0f);
        
        if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }
        
        if (!m_depthBuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT, true, GL_NONE)) {
            return false;
        }

        if (!m_aoBuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT, false, GL_R32F)) {
            return false;
        }

        if (!m_blurBuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT, false, GL_R32F)) {
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
        OgldevBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        m_pGameCamera->OnRender();      

        m_pipeline.SetCamera(*m_pGameCamera);
		
        GeometryPass();
        
        SSAOPass();
        
        BlurPass();
        
        LightingPass();                
		      	
        RenderFPS();     
        
        CalcFPS();
        
        OgldevBackendSwapBuffers();
    }
    
    void GeometryPass()
    {
		m_geomPassTech.Enable();        

        m_depthBuffer.BindForWriting();

        glClear(GL_DEPTH_BUFFER_BIT);

        m_pipeline.Orient(m_mesh.GetOrientation());
        m_geomPassTech.SetWVP(m_pipeline.GetWVPTrans());
        m_mesh.Render();       
    }
    
      
    void SSAOPass()
    {
        m_SSAOTech.Enable();        
        m_SSAOTech.BindDepthBuffer(m_depthBuffer);        
        
        m_aoBuffer.BindForWriting();
        
        glClear(GL_COLOR_BUFFER_BIT);                
                  
        m_quad.Render();                
    }

    
    void BlurPass()
    {
        m_blurTech.Enable();
        
        m_blurTech.BindInputBuffer(m_aoBuffer);
        
        m_blurBuffer.BindForWriting();
        
        glClear(GL_COLOR_BUFFER_BIT);                
        
        m_quad.Render();                
    }

    
    void LightingPass()
    {
        m_lightingTech.Enable();
        m_lightingTech.SetShaderType(m_shaderType);                
        m_lightingTech.BindAOBuffer(m_blurBuffer);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);        
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
        m_pipeline.Orient(m_mesh.GetOrientation());
        m_lightingTech.SetWVP(m_pipeline.GetWVPTrans());        
        m_lightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());        
        m_mesh.Render();               
    }
    
             
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
    {
        switch (OgldevKey) {
            case OGLDEV_KEY_ESCAPE:
            case OGLDEV_KEY_q:
                OgldevBackendLeaveMainLoop();
                break;
            case OGLDEV_KEY_A:
                m_shaderType++;
                m_shaderType = m_shaderType % 3;
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
        
    SSAOTechnique m_SSAOTech;
    GeomPassTech m_geomPassTech;
    LightingTechnique m_lightingTech;
    BlurTech m_blurTech;
    Camera* m_pGameCamera;
    Mesh m_mesh;
    Mesh m_quad;
    PersProjInfo m_persProjInfo;
    Pipeline m_pipeline;
    IOBuffer m_depthBuffer;
    IOBuffer m_aoBuffer;
    IOBuffer m_blurBuffer;
    DirectionalLight m_directionalLight;
    int m_shaderType;
};


int main(int argc, char** argv)
{
  //  Magick::InitializeMagick(*argv);

    OgldevBackendInit(OGLDEV_BACKEND_TYPE_GLFW, argc, argv, true, false);

    if (!OgldevBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 45")) {
        OgldevBackendTerminate();
		return 1;
    }

    SRANDOM;
    
    Tutorial46* pApp = new Tutorial46();

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
