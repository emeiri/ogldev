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
//#include "ogldev_basic_lighting.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_shadow_map_fbo.h"
#include "lighting_technique.h"
#include "shadow_map_technique.h"


#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 1024

class Tutorial47 : public ICallbacks, public OgldevApp
{
public:

    Tutorial47() 
    {
        m_pGameCamera = NULL;
        m_pGroundTex  = NULL;
        
        m_spotLight.AmbientIntensity = 0.0f;
        m_spotLight.DiffuseIntensity = 0.9f;
        m_spotLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_spotLight.Attenuation.Linear = 0.0f;
        m_spotLight.Position  = Vector3f(-500.0, 500.0, 25.0f);
        m_spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.Cutoff =  30.0f;
        
        m_dirLight.AmbientIntensity = 0.0f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.Direction = Vector3f(0.0f, -1.0f, 0.0f);

        m_persProjInfo.FOV    = 45.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width  = WINDOW_WIDTH;
        m_persProjInfo.zNear  = 1.0f;
        m_persProjInfo.zFar   = 1000.0f;  

        for (int i = 0 ; i < NUM_SHADOW_CASCADES ; i++) {
            m_shadowPersProjInfo[i].FOV    = 45.0f;
            m_shadowPersProjInfo[i].Height = 1024;
            m_shadowPersProjInfo[i].Width  = 1024;            
        }

        m_shadowPersProjInfo[0].zNear = 1.0f;                    
        m_shadowPersProjInfo[0].zFar  = 500.0f;  
        m_shadowPersProjInfo[1].zNear = 1.0f;
        m_shadowPersProjInfo[1].zFar  = 1000.0f; 
        m_shadowPersProjInfo[2].zNear = 1.0f;
        m_shadowPersProjInfo[2].zFar  = 100.0f; 
        m_shadowPersProjInfo[3].zNear = 1.0f;
        m_shadowPersProjInfo[3].zFar  = 1000.0f;
    }

    ~Tutorial47()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pGroundTex);
    }    

    bool Init()
    {
        if (!m_shadowMapFBO.Init(1024, 1024)) {
            return false;
        }

        Vector3f Pos(3.0f, 8.0f, -10.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
        
        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
        
        if (!m_LightingTech.Init()) {
            OGLDEV_ERROR("Error initializing the lighting technique\n");
            return false;
        }

        m_LightingTech.Enable();

        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetSpotLights(1, &m_spotLight);
        m_LightingTech.SetDirectionalLight(m_dirLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);
        m_LightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);		
        
        for (int i = 0 ; i < NUM_SHADOW_CASCADES ; i++) {
            Matrix4f LightProj;
            LightProj.InitPersProjTransform(m_shadowPersProjInfo[i]);
            m_LightingTech.SetLightProj(i, LightProj, m_shadowPersProjInfo[i].zFar);
        }        

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
        m_pGameCamera->OnRender();
        
        ShadowMapPass();
        RenderPass();
     
        OgldevBackendSwapBuffers();
    }
	
	
    void ShadowMapPass()
    {
        m_ShadowMapEffect.Enable();
       
        m_shadowMapFBO.BindForWriting();
        glClear(GL_DEPTH_BUFFER_BIT);

        Pipeline p;
      //  p.Scale(0.1f, 0.1f, 0.1f);
        p.WorldPos(0.0f, 0.0f, 3.0f);
        p.SetCamera(m_spotLight.Position, m_spotLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        p.SetPerspectiveProj(m_shadowPersProjInfo[0]);                    
        m_ShadowMapEffect.SetWVP(p.GetWVPTrans());
        m_mesh.Render();        
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

        
    void RenderPass()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_LightingTech.Enable();

        m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());
       
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

        Pipeline p;
        
        p.SetPerspectiveProj(m_persProjInfo);
        p.Scale(500.0f, 500.0f, 500.0f);
        p.WorldPos(0.0f, 0.0f, 10.0f);
        p.Rotate(90.0f, 0.0f, 0.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        p.SetCamera(m_spotLight.Position, m_spotLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        m_LightingTech.SetLightWV(p.GetWVTrans());
        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);
        m_quad.Render();
 
        p.Scale(1.0f, 1.0f, 1.0f);
		p.Rotate(0.0f, 0.0f, 0.0f);
        p.SetCamera(m_spotLight.Position, m_spotLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        m_LightingTech.SetLightWV(p.GetWVTrans());

        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        
        for (int i = 0; i < 4 ; i++) {
            p.WorldPos(0.0f, 0.0f, 3.0f + i * 30.0f);
            m_LightingTech.SetWVP(p.GetWVPTrans());
            m_LightingTech.SetWorldMatrix(p.GetWorldTrans());
            m_mesh.Render();
        }
    }
	
	

    /*void RenderPass()
    {   
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
        m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());
        
        m_pipeline.SetCamera(*m_pGameCamera);
        
        float ViewSpaceDepth[] = { 5.0f, 50.0f };
        
        float ClipSpaceDepth[2];
        Matrix4f PersProjTrans;
        PersProjTrans.InitPersProjTransform(m_persProjInfo);

        for (int i = 0 ; i < 2 ; i++) {
            printf("\n\n%i\n", i);
            Vector4f v = PersProjTrans * Vector4f(0.0f, 0.0f, ViewSpaceDepth[i], 1.0f);
            v.Print();
            printf("\n");
            ClipSpaceDepth[i] = v.z / v.w;
            printf("clip space depth %f\n", ClipSpaceDepth[i]);
        }
        
        Matrix4f ViewProjInverse = m_pipeline.GetVPTrans();
        ViewProjInverse.Inverse();
        
        Vector4f Corners[] = {  Vector4f(-1.0f, -1.0f, ClipSpaceDepth[0], 1.0f),
                                Vector4f(1.0f, -1.0f,  ClipSpaceDepth[0], 1.0f),
                                Vector4f(-1.0f, 1.0f,  ClipSpaceDepth[0], 1.0f),
                                Vector4f(1.0f, 1.0f,   ClipSpaceDepth[0], 1.0f),
                                Vector4f(-1.0f, -1.0f, ClipSpaceDepth[1], 1.0f),
                                Vector4f(1.0f, -1.0f,  ClipSpaceDepth[1], 1.0f),
                                Vector4f(-1.0f, 1.0f,  ClipSpaceDepth[1], 1.0f),
                                Vector4f(1.0f, 1.0f,   ClipSpaceDepth[1], 1.0f) };
        
        Vector3f ClipCorners[8];
        
        for (int i = 0 ; i < 8 ; i++) {
            Vector4f v = ViewProjInverse * Corners[i];                    
            ClipCorners[i] = (v / v.w).to3f();
            ClipCorners[i].Print(); printf("\n");
        }
        
        exit(0);
            
        m_LightingTech.SetWVP(m_pipeline.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());            
       
        m_mesh.Render();        
        	
    //    RenderFPS();     
        CalcFPS();        
    }*/
    
       
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
    ShadowMapTechnique m_ShadowMapEffect;
    Camera* m_pGameCamera;
    SpotLight m_spotLight;
    DirectionalLight m_dirLight;
    BasicMesh m_mesh;
	BasicMesh m_quad;
    Texture* m_pGroundTex;
    ShadowMapFBO m_shadowMapFBO;
    PersProjInfo m_persProjInfo;
    PersProjInfo m_shadowPersProjInfo[NUM_SHADOW_CASCADES];
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
