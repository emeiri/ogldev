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

    Tutorial 47 - Shadow Mapping With Directional Lights
*/

#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_shadow_map_fbo.h"
#include "lighting_technique.h"
#include "shadow_map_technique.h"

#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 1024

#define NUM_MESHES 5

class Tutorial47 : public ICallbacks, public OgldevApp
{
public:

    Tutorial47() 
    {
        m_pGameCamera = NULL;
        m_pGroundTex  = NULL;
        
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);

        m_persProjInfo.FOV    = 45.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width  = WINDOW_WIDTH;
        m_persProjInfo.zNear  = 1.0f;
        m_persProjInfo.zFar   = 1000.0f;  
 
        m_shadowOrthoProjInfo.l = -100.0f;
        m_shadowOrthoProjInfo.r = 100.0f;
        m_shadowOrthoProjInfo.t = 100.0f;
        m_shadowOrthoProjInfo.b = -100.0f;
        m_shadowOrthoProjInfo.n = -10.0f;                    
        m_shadowOrthoProjInfo.f = 100.0f;          
        
        m_quad.GetOrientation().m_scale    = Vector3f(50.0f, 100.0f, 100.0f);
        m_quad.GetOrientation().m_pos      = Vector3f(0.0f, 0.0f, 90.0f);
        m_quad.GetOrientation().m_rotation = Vector3f(90.0f, 0.0f, 0.0f);

        for (int i = 0; i < NUM_MESHES ; i++) {
            m_meshOrientation[i].m_scale    = Vector3f(1.0f, 1.0f, 1.0f);
            m_meshOrientation[i].m_pos      = Vector3f(0.0f, 0.0f, 3.0f + i * 30.0f);
        }                    
    }

    ~Tutorial47()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pGroundTex);
    }    

    bool Init()
    {
        if (!m_shadowMapFBO.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

        Vector3f Pos(8.0, 21.0, -23.0);
        Vector3f Target(-0.07f, -0.44f, 0.9f);
        Vector3f Up(0.0, 1.0f, 0.0f);
        
        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
        
        if (!m_LightingTech.Init()) {
            OGLDEV_ERROR0("Error initializing the lighting technique\n");
            return false;
        }

        m_LightingTech.Enable();

        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);		
        m_LightingTech.SetDirectionalLight(m_dirLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);
                
        if (!m_mesh.LoadMesh("../Content/dragon.obj")) {
            return false;            
        }                
       
        if (!m_ShadowMapEffect.Init()) {
            printf("Error initializing the shadow map technique\n");
            return false;
        }        
	            
#ifndef WIN32
        // Disabled for now because it somehow clashes with the regular rendering...
 //       if (!m_fontRenderer.InitFontRenderer()) {
   //         return false;
   //     }
#endif        	      
		if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }

		m_pGroundTex = new Texture(GL_TEXTURE_2D, "../Content/wal67ar_small.jpg");

        if (!m_pGroundTex->Load()) {
            return false;
        }
        
        return true;
    }

    
    void Run()
    {
        OgldevBackendRun(this);
    }

    
    virtual void RenderSceneCB()
    {
        for (int i = 0; i < NUM_MESHES ; i++) {
            m_meshOrientation[i].m_rotation.y += 0.5f;
        }
        
        m_pGameCamera->OnRender();
        
        ShadowMapPass();
        RenderPass();
     
        OgldevBackendSwapBuffers();
    }
	

    void ShadowMapPass()
    {      
        m_shadowMapFBO.BindForWriting();
        glClear(GL_DEPTH_BUFFER_BIT);

        m_ShadowMapEffect.Enable();

        Pipeline p;
        p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        p.SetOrthographicProj(m_shadowOrthoProjInfo);                    
        
        for (int i = 0; i < NUM_MESHES ; i++) {
            p.Orient(m_meshOrientation[i]);
            m_ShadowMapEffect.SetWVP(p.GetWVOrthoPTrans());
            m_mesh.Render();
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

        
    void RenderPass()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_LightingTech.Enable();

        m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());
       
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

        Pipeline p;        
        p.SetOrthographicProj(m_shadowOrthoProjInfo);        
        p.Orient(m_quad.GetOrientation());
        p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        m_LightingTech.SetLightWVP(p.GetWVOrthoPTrans());        
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);        
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);
        m_quad.Render();
           
        for (int i = 0; i < NUM_MESHES ; i++) {
            p.Orient(m_meshOrientation[i]);
            m_LightingTech.SetWVP(p.GetWVPTrans());
            m_LightingTech.SetWorldMatrix(p.GetWorldTrans());
            m_mesh.Render();
        }
    }
    
	       
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE OgldevKeyState = OGLDEV_KEY_STATE_PRESS)
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
    ShadowMapTechnique m_ShadowMapEffect;
    Camera* m_pGameCamera;
    DirectionalLight m_dirLight;
    BasicMesh m_mesh;
    Orientation m_meshOrientation[NUM_MESHES];
	BasicMesh m_quad;
    Texture* m_pGroundTex;
    ShadowMapFBO m_shadowMapFBO;
    PersProjInfo m_persProjInfo;
    OrthoProjInfo m_shadowOrthoProjInfo;
};


int main(int argc, char** argv)
{
    OgldevBackendInit(OGLDEV_BACKEND_TYPE_GLFW, argc, argv, true, false);

    if (!OgldevBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 47")) {
        OgldevBackendTerminate();
		return 1;
    }

    SRANDOM;
    
    Tutorial47* pApp = new Tutorial47();

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
