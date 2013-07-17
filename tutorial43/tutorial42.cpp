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

    Tutorial 42 - Screen Space Ambient Occlusion
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "engine_common.h"
#include "util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "motion_blur_technique.h"
#include "glut_backend.h"
#include "mesh.h"
#include "intermediate_buffer.h"
#ifndef WIN32
#include "freetypeGL.h"
#endif

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

#ifndef WIN32
Markup sMarkup = { (char*)"Arial", 64, 1, 0, 0.0, 0.0,
                   {.1,1.0,1.0,.5}, {1,1,1,0},
                   0, {1,0,0,1}, 0, {1,0,0,1},
                   0, {0,0,0,1}, 0, {0,0,0,1} };
#endif

#ifdef WIN32
static long long GetCurrentTimeMillis()
{
	return GetTickCount();
}
#else
static long long GetCurrentTimeMillis()
{
	timeval t;
	gettimeofday(&t, NULL);

	long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;

	return ret;
}
#endif


class Tutorial42 : public ICallbacks
{
public:

    Tutorial42() 
    {
        m_pGameCamera = NULL;
        m_pLightingTech = NULL;
        m_pMotionBlurTech = NULL;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.66f;
        m_directionalLight.DiffuseIntensity = 1.0f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_frameCount = 0;
        m_fps = 0.0f;
        
        m_position = Vector3f(0.0f, 0.0f, 10.0f);      
    }

    ~Tutorial42()
    {
        SAFE_DELETE(m_pLightingTech);
        SAFE_DELETE(m_pMotionBlurTech);
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 3.0f, -1.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
        
        if (!m_intermediateBuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            printf("Error initializing the intermediate buffer\n");
            return false;
        }
      
        m_pLightingTech = new LightingTechnique();

        if (!m_pLightingTech->Init()) {
            printf("Error initializing the skinning technique\n");
            return false;
        }

        m_pLightingTech->Enable();

        m_pLightingTech->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pLightingTech->SetDirectionalLight(m_directionalLight);
        m_pLightingTech->SetMatSpecularIntensity(0.0f);
        m_pLightingTech->SetMatSpecularPower(0);

        m_pMotionBlurTech = new MotionBlurTechnique();

        if (!m_pMotionBlurTech->Init()) {
            printf("Error initializing the motion blur technique\n");
            return false;
        }
        
        m_pMotionBlurTech->Enable();
        m_pMotionBlurTech->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pMotionBlurTech->SetMotionTextureUnit(MOTION_TEXTURE_UNIT_INDEX);

        if (!m_mesh.LoadMesh("models/graving-dock.3ds")) {
            printf("Mesh load failed\n");
            return false;            
        }
        
        if (!m_quad.LoadMesh("models/quad.obj")) {
            printf("Quad mesh load failed\n");
            return false;            
        }
        
#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif        	
        m_glutTime = glutGet(GLUT_ELAPSED_TIME);
        m_startTime = GetCurrentTimeMillis();
       
        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        CalcFPS();
              
        m_pGameCamera->OnRender();

        RenderDepthPass();
        
        //MotionBlurPass();
                              
        RenderFPS();       
        
        glutSwapBuffers();
    }
    
    void RenderDepthPass()
    {
        m_intermediateBuffer.BindForDepthPass();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pLightingTech->Enable();
                                   
        m_pLightingTech->SetEyeWorldPos(m_pGameCamera->GetPos());
        
        m_pipeline.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        m_pipeline.SetPerspectiveProj(m_persProjInfo);           
        m_pipeline.Scale(0.02f, 0.02f, 0.02f);                
             
        Vector3f Pos(m_position);
        m_pipeline.WorldPos(Pos);        
 //       m_pipeline.Rotate(270.0f, 180.0f, 0.0f);       
        m_pLightingTech->SetWVP(m_pipeline.GetWVPTrans());
        m_pLightingTech->SetWorldMatrix(m_pipeline.GetWorldTrans());            
       
        m_mesh.Render();        
    }
    
    
    void MotionBlurPass()
    {
        m_intermediateBuffer.BindForReading();

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        m_pMotionBlurTech->Enable();
       
        m_quad.Render();
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
     
    LightingTechnique* m_pLightingTech;
    MotionBlurTechnique* m_pMotionBlurTech;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
    Mesh m_mesh;
    Mesh m_quad;
    Vector3f m_position;            
    PersProjInfo m_persProjInfo;
    IntermediateBuffer m_intermediateBuffer;
    Pipeline m_pipeline;
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
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 42")) {
        return 1;
    }
    
    SRANDOM;
    
    Tutorial42* pApp = new Tutorial42();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}