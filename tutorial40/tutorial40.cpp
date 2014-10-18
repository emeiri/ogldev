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

    Tutorial 40 - Stencil Shadow Volume
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#include "engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "texture.h"
#include "shadow_volume_technique.h"
#include "lighting_technique.h"
#include "ogldev_glut_backend.h"
#include "mesh.h"
#include "null_technique.h"

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024


class Tutorial40 : public ICallbacks, public OgldevApp
{
public:

    Tutorial40() 
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;
        m_pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLight.Position = Vector3f(0.0f, 10.0f, 0.0f);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 1000.0f;  
        
        m_boxPos = Vector3f(0.0f, 2.0f, 0.0);
        
        m_isWireframe = false;
    }

    ~Tutorial40()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 20.0f, -7.0f);
        Vector3f Target(0.0f, -1.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
        
        if (!m_nullTech.Init()) {
            printf("Error initializing the null technique\n");
            return false;            
        }
      
        if (!m_ShadowVolTech.Init()) {
            printf("Error initializing the shadow volume technique\n");
            return false;
        }

        if (!m_LightingTech.Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }
        
        m_LightingTech.Enable();
        
        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetPointLights(1, &m_pointLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);        

        if (!m_box.LoadMesh("../Content/box.obj", true)) {
            printf("Mesh load failed\n");
            return false;            
        }
        
#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif        	              
        if (!m_quad.LoadMesh("../Content/quad.obj", false)) {
            return false;
        }

        m_pGroundTex = new Texture(GL_TEXTURE_2D, "../Content/test.png");

        if (!m_pGroundTex->Load()) {
            return false;
        }
                     
        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
         

    virtual void RenderSceneCB()
    {   
        CalcFPS();
        
        m_scale += 0.1f;
               
        m_pGameCamera->OnRender();
        
        glDepthMask(GL_TRUE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
               
        RenderSceneIntoDepth();
        
        glEnable(GL_STENCIL_TEST);
                
        RenderShadowVolIntoStencil();
                
        RenderShadowedScene();
        
        glDisable(GL_STENCIL_TEST);
        
        RenderAmbientLight();
        
   //     RenderFPS();
        
        glutSwapBuffers();
    }


	virtual void KeyboardCB(OGLDEV_KEY OgldevKey)
	{
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
			break;
        case OGLDEV_KEY_w:
            m_isWireframe = !m_isWireframe;
            if (m_isWireframe) {                
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }                            
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
    
    void RenderSceneIntoDepth()
    {
        glDrawBuffer(GL_NONE);
              
        m_nullTech.Enable();

        Pipeline p;
        
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);                       
                
        p.WorldPos(m_boxPos);        
        p.Rotate(0, m_scale, 0);        
        m_nullTech.SetWVP(p.GetWVPTrans());        
        m_box.Render();                
        
        p.Scale(10.0f, 10.0f, 10.0f);
        p.WorldPos(0.0f, 0.0f, 0.0f);
        p.Rotate(90.0f, 0.0f, 0.0f);
        m_nullTech.SetWVP(p.GetWVPTrans());
        m_quad.Render();             
    }

    void RenderShadowVolIntoStencil()
    {
        glDrawBuffer(GL_NONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_CLAMP);        
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
                    
		// We need the stencil test to be enabled but we want it
		// to succeed always. Only the depth test matters.
		glStencilFunc(GL_ALWAYS, 0, 0xff);

        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);       
		        
        m_ShadowVolTech.Enable();

        m_ShadowVolTech.SetLightPos(m_pointLight.Position);
               
        Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());        
        p.SetPerspectiveProj(m_persProjInfo);                       
        p.WorldPos(m_boxPos);        
        p.Rotate(0, m_scale, 0);
        m_ShadowVolTech.SetWorldMatrix(p.GetWorldTrans());
        Matrix4f Identity;
        Identity.InitIdentity();
        m_ShadowVolTech.SetWorldMatrix(Identity);
        
        m_ShadowVolTech.SetVP(p.GetWVPTrans());
        
        m_box.Render();        
        
        glDisable(GL_DEPTH_CLAMP);
        glEnable(GL_CULL_FACE);                  
        glDepthFunc(GL_LEQUAL);
    }
        
    void RenderShadowedScene()
    {
        glDrawBuffer(GL_BACK);
                                          
        // prevent update to the stencil buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_EQUAL, 0x0, 0xFF);
        
        m_LightingTech.Enable();
        
        m_pointLight.AmbientIntensity = 0.0f;
        m_pointLight.DiffuseIntensity = 0.8f;

        m_LightingTech.SetPointLights(1, &m_pointLight);
                                     
        Pipeline p;
        p.SetPerspectiveProj(m_persProjInfo);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        
        p.WorldPos(m_boxPos);
        p.Rotate(0, m_scale, 0);
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_box.Render();
       
        p.Scale(10.0f, 10.0f, 10.0f);
        p.WorldPos(0.0f, 0.0f, 0.0f);
        p.Rotate(90.0f, 0.0f, 0.0f);
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);
        m_quad.Render();        
    }    
    
    
    void RenderAmbientLight()
    {
        glDrawBuffer(GL_BACK);
        glDepthMask(GL_TRUE);
        
     	glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
                                            
        m_LightingTech.Enable();

        m_pointLight.AmbientIntensity = 0.2f;
        m_pointLight.DiffuseIntensity = 0.0f;

        m_LightingTech.SetPointLights(1, &m_pointLight);
                             
        Pipeline p;
        p.SetPerspectiveProj(m_persProjInfo);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        
        p.WorldPos(m_boxPos);
        p.Rotate(0, m_scale, 0);
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_box.Render();
       
        p.Scale(10.0f, 10.0f, 10.0f);
        p.WorldPos(0.0f, 0.0f, 0.0f);
        p.Rotate(90.0f, 0.0f, 0.0f);
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());

        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);        
        m_quad.Render();        
        
        glDisable(GL_BLEND);
    }        
   
    ShadowVolumeTechnique m_ShadowVolTech;
    LightingTechnique m_LightingTech;
    NullTechnique m_nullTech;
    Camera* m_pGameCamera;
    float m_scale;
    PointLight m_pointLight;
    Vector3f m_boxPos;
    Mesh m_box;
    Mesh m_quad;
    Texture* m_pGroundTex;
    PersProjInfo m_persProjInfo;
    bool m_isWireframe;
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv, true, true);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 40")) {
        return 1;
    }
    
    SRANDOM;

	glDepthFunc(GL_LEQUAL);
    
    Tutorial40* pApp = new Tutorial40();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}