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
#include "ogldev_util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "shadow_volume_technique.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "mesh.h"
#ifndef WIN32
#include "freetypeGL.h"
#endif
#include "null_technique.h"

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

#ifndef WIN32
Markup sMarkup = { (char*)"Arial", 64, 1, 0, 0.0, 0.0,
                   {.1,1.0,1.0,.5}, {1,1,1,0},
                   0, {1,0,0,1}, 0, {1,0,0,1},
                   0, {0,0,0,1}, 0, {0,0,0,1} };
#endif

class Tutorial40 : public ICallbacks
{
public:

    Tutorial40() 
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;
        m_pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLight.Position = Vector3f(0.0f, 15.0f, 0.0f);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_frameCount = 0;
        m_fps = 0.0f;
        
        m_boxPos = Vector3f(0.0f, 2.0f, 0.0);
    }

    ~Tutorial40()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 2.0f, -7.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
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
        m_glutTime = glutGet(GLUT_ELAPSED_TIME);
        m_startTime = GetCurrentTimeMillis();
               
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
               
        RenderSceneIntoDepth();
        
        glEnable(GL_STENCIL_TEST);
                
        RenderShadowVolIntoStencil();
        
        RenderShadowedScene();
        
        glDisable(GL_STENCIL_TEST);
        
        RenderAmbientLight();
        
        RenderFPS();
        
        glutSwapBuffers();
    }

    virtual void IdleCB()
    {
        RenderSceneCB();
    }

    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        m_pGameCamera->OnKeyboard(Key);
    }


    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key) {
            case 'q':
                glutLeaveMainLoop();
                break;
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }
    
    
    virtual void MouseCB(int Button, int State, int x, int y)
    {
    }


private:
    
    void RenderSceneIntoDepth()
    {
        glDrawBuffer(GL_NONE);
        glDepthMask(GL_TRUE);
              
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
        
        glDisable(GL_CULL_FACE);
                    
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
        m_ShadowVolTech.SetVP(p.GetVPTrans());
        
        m_box.Render();        
        
        glEnable(GL_CULL_FACE);                  
    }
        
    void RenderShadowedScene()
    {
        glDrawBuffer(GL_BACK);
                                    
        // prevent update to the stencil buffer
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
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

        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);
                              
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
        
        m_quad.Render();        
        
        glDisable(GL_BLEND);
    }        

    
    void CalcFPS()
    {
        m_frameCount++;
        
        int time = glutGet( GLUT_ELAPSED_TIME );

        if (time - m_glutTime > 1000) {
            m_fps = (float)m_frameCount * 1000.0f / (time - m_glutTime);
            m_glutTime = time;
            m_frameCount = 0;
        }
    }
        
    void RenderFPS()
    {
        char text[32];
        ZERO_MEM(text);        
        SNPRINTF(text, sizeof(text), "FPS: %.2f", m_fps);
#ifndef WIN32
        m_fontRenderer.RenderText(10, 10, text);        
#endif
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
#ifndef WIN32
    FontRenderer m_fontRenderer;
#endif
    int m_glutTime;
    long long m_startTime;
    int m_frameCount;
    float m_fps;    
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 40")) {
        return 1;
    }
    
    SRANDOM;
    
    Tutorial40* pApp = new Tutorial40();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}