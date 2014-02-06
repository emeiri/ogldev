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

    Tutorial 38 - Skinning
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
#include "ogldev_util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "skinning_technique.h"
#include "glut_backend.h"
#include "mesh.h"
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

class Tutorial38 : public ICallbacks
{
public:

    Tutorial38() 
    {
        m_pGameCamera = NULL;
        m_pEffect = NULL;
        m_scale = 0.0f;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.55f;
        m_directionalLight.DiffuseIntensity = 0.9f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_frameCount = 0;
        m_fps = 0.0f;
        
        m_position = Vector3f(0.0f, 0.0f, 6.0f);      
    }

    ~Tutorial38()
    {
        SAFE_DELETE(m_pEffect);
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 3.0f, -1.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        m_pEffect = new SkinningTechnique();

        if (!m_pEffect->Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_pEffect->Enable();

        m_pEffect->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pEffect->SetDirectionalLight(m_directionalLight);
        m_pEffect->SetMatSpecularIntensity(0.0f);
        m_pEffect->SetMatSpecularPower(0);

        if (!m_mesh.LoadMesh("../Content/boblampclean.md5mesh")) {
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
        
        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        CalcFPS();
        
        m_scale += 0.005f;
        
        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pEffect->Enable();
        
        vector<Matrix4f> Transforms;
               
        float RunningTime = (float)((double)GetCurrentTimeMillis() - (double)m_startTime) / 1000.0f;

        m_mesh.BoneTransform(RunningTime, Transforms);
        
        for (uint i = 0 ; i < Transforms.size() ; i++) {
            m_pEffect->SetBoneTransform(i, Transforms[i]);
        }
        
        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
        
        Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);           
        p.Scale(0.1f, 0.1f, 0.1f);                
             
        Vector3f Pos(m_position);
        p.WorldPos(Pos);        
        p.Rotate(270.0f, 180.0f, 0.0f);       
        m_pEffect->SetWVP(p.GetWVPTrans());
        m_pEffect->SetWorldMatrix(p.GetWorldTrans());            

        m_mesh.Render();
                              
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
    
 
    SkinningTechnique* m_pEffect;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
    Mesh m_mesh;
    Vector3f m_position;            
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

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 38")) {
        return 1;
    }
    
    SRANDOM;
    
    Tutorial38* pApp = new Tutorial38();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}