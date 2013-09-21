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

    Tutorial 43 - Shadow mapping with point lights
*/
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <assert.h>

#include "engine_common.h"
#include "util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "mesh.h"
#include "shadow_map_technique.h"
#include "shadow_map_fbo.h"
#ifndef WIN32
#include "freetypeGL.h"
#endif
using namespace std;

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


struct CameraDirection
{
    GLenum CubemapFace;
    Vector3f Target;
    Vector3f Up;
};

CameraDirection gCameraDirections[NUM_OF_LAYERS] = 
{
    { GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 0.0f, -1.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, Vector3f(0.0f, -1.0f, 0.0f),  Vector3f(0.0f, 0.0f, 1.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, Vector3f(0.0f, 0.0f, -1.0f),  Vector3f(0.0f, 1.0f, 0.0f) }
};

#ifndef WIN32
Markup sMarkup = { (char*)"Arial", 64, 1, 0, 0.0, 0.0,
                   {.1,1.0,1.0,.5}, {1,1,1,0},
                   0, {1,0,0,1}, 0, {1,0,0,1},
                   0, {0,0,0,1}, 0, {0,0,0,1} };
#endif

class Tutorial43 : public ICallbacks
{
public:

    Tutorial43() 
#ifndef WIN32
           : m_fontRenderer2(sMarkup)
#endif
    {
        m_pGameCamera = NULL;
        m_pLightingEffect = NULL;
        m_pShadowMapEffect = NULL;
        m_scale = 0.0f;
        m_pointLight.AmbientIntensity = 0.1f;
        m_pointLight.DiffuseIntensity = 0.9f;
        m_pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLight.Attenuation.Linear = 0.0f;
        m_pointLight.Position  = Vector3f(0.0, 0.0, .0f);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_frameCount = 0;
        m_fps = 0.0f;
    }

    ~Tutorial43()
    {
        SAFE_DELETE(m_pLightingEffect);
        SAFE_DELETE(m_pShadowMapEffect);
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(3.0f, 8.0f, -10.0f);
        Vector3f Target(0.0f, -0.5f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        if (!m_shadowMapFBO.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            printf("Error initializing the shadow map FBO\n");
            return false;
        }

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        m_pLightingEffect = new LightingTechnique();

        if (!m_pLightingEffect->Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_pLightingEffect->Enable();

        m_pLightingEffect->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pLightingEffect->SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);		
        m_pLightingEffect->SetPointLights(1, &m_pointLight);
        m_pLightingEffect->SetShadowMapSize((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);

        m_pShadowMapEffect = new ShadowMapTechnique();

        if (!m_pShadowMapEffect->Init()) {
            printf("Error initializing the shadow map technique\n");
            return false;
        }        

		if (!m_quad.LoadMesh("models/quad.obj")) {
            return false;
        }

		m_pGroundTex = new Texture(GL_TEXTURE_2D, "models/checkers.png");

        if (!m_pGroundTex->Load()) {
            return false;
        }

        
        if (!m_mesh.LoadMesh("../tutorial25/sphere.obj")) {
	//	if (!m_mesh.LoadMesh("models/phoenix_ugv.md2")) {
			return false;
		}

#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
        
        if (!m_fontRenderer2.InitFontRenderer()) {
            return false;
        }
#endif
        
        m_time = glutGet(GLUT_ELAPSED_TIME);
        
        glEnable(GL_TEXTURE_CUBE_MAP);
        
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

        ShadowMapPass();
    //    RenderPass();
        
   //     RenderFPS();

        glutSwapBuffers();
    }

    void ShadowMapPass()
    {
        glCullFace(GL_FRONT);
        
        m_pShadowMapEffect->Enable();
        GLExitIfError();        
        Pipeline p;
        
        PersProjInfo Info;
        Info.FOV = 90.0f;
        Info.Height = WINDOW_HEIGHT;
        Info.Width = WINDOW_WIDTH;
        Info.zNear = 1.0f;
        Info.zFar = 100.0f;  

        p.SetPerspectiveProj(Info);
        p.Scale(1.0f);
        p.Rotate(0.0f, 0.0f, 0.0f);
        p.WorldPos(0.0f, 2.0f, 5.0f);
                       
        for (uint i = 0 ; i < NUM_OF_LAYERS ; i++) {
            p.SetCamera(m_pointLight.Position, gCameraDirections[i].Target, gCameraDirections[i].Up);
            m_pShadowMapEffect->SetWVP(p.GetWVPTrans());
            GLExitIfError();        

            m_shadowMapFBO.BindForWriting(gCameraDirections[i].CubemapFace);
            glClear(GL_DEPTH_BUFFER_BIT);
            m_mesh.Render();
        }        
        
        GLExitIfError();        
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                GLExitIfError();        
     //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                GLExitIfError();        
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);        
//        m_shadowMapFBO.Dump();
                GLExitIfError();        
     //   glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
         //       GLExitIfError();        
                
                exit(0);
    }
        
    void RenderPass()
    {
        glCullFace(GL_BACK);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pLightingEffect->Enable();
             
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

        Pipeline p;
        p.SetPerspectiveProj(m_persProjInfo);
        m_pLightingEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
        
        // Render the quad
        p.Scale(10.0f);
        p.WorldPos(0.0f, 0.0f, 1.0f);
        p.Rotate(90.0f, 0.0f, 0.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());                
        m_pLightingEffect->SetWVP(p.GetWVPTrans());
        m_pLightingEffect->SetWorldMatrix(p.GetWorldTrans());
        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);
        m_quad.Render();
        GLExitIfError();        
             
        // Render the object
        p.Scale(1.0f);
        p.Rotate(0.0f, m_scale, 0.0f);
        p.WorldPos(0.0f, 2.0f, 5.0f);
        m_pLightingEffect->SetWVP(p.GetWVPTrans());
        m_pLightingEffect->SetWorldMatrix(p.GetWorldTrans());
        m_mesh.Render();        
        GLExitIfError();        
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
    
    void CalcFPS()
    {
        m_frameCount++;
        
        int time = glutGet( GLUT_ELAPSED_TIME );

        if (time - m_time > 1000) {
            m_fps = (float)m_frameCount * 1000.0f / (time - m_time);
            m_time = time;
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

    LightingTechnique* m_pLightingEffect;
    ShadowMapTechnique* m_pShadowMapEffect;
    Camera* m_pGameCamera;
    float m_scale;
    PointLight m_pointLight;
    Mesh m_mesh;
    Mesh m_quad;	
    PersProjInfo m_persProjInfo;
    Texture* m_pGroundTex;
    ShadowMapFBO m_shadowMapFBO;
    int m_time;
    int m_frameCount;
    float m_fps;    
#ifndef WIN32
    FontRenderer m_fontRenderer;
    FontRenderer m_fontRenderer2;
#endif
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 43")) {
        return 1;
    }

    Tutorial43 App;

    if (!App.Init()) {
        return 1;
    }

    App.Run();

    return 0;
}