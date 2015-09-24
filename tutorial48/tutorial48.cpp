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

    Tutorial 48 - Ant Tweak Bar
*/

#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <AntTweakBar.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ogldev_basic_lighting.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "ogldev_basic_mesh.h"

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

float g_Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };

typedef enum { BUDDHA, BUNNY, DRAGON } MESH_TYPE;

void TW_CALL RunCB(void *p /*clientData*/)
{ 
}


class ATB
{
public:
    ATB();
    
    bool Init();
    
    bool KeyboardCB(OGLDEV_KEY OgldevKey);

    bool PassiveMouseCB(int x, int y);

    void RenderSceneCB();

    bool MouseCB(OGLDEV_MOUSE Button, OGLDEV_KEY_STATE State, int x, int y);
    
};


ATB::ATB()
{
    
}


bool ATB::Init()
{
    return (TwInit(TW_OPENGL, NULL) == 1);
}

bool ATB::KeyboardCB(OGLDEV_KEY OgldevKey)
{
    return (TwKeyPressed(OgldevKey, TW_KMOD_NONE) == 1);
}


bool ATB::PassiveMouseCB(int x, int y)
{
    return (TwMouseMotion(x, y) == 1);
}


bool ATB::MouseCB(OGLDEV_MOUSE Button, OGLDEV_KEY_STATE State, int x, int y)
{    
    TwMouseButtonID btn = (Button == OGLDEV_MOUSE_BUTTON_LEFT) ? TW_MOUSE_LEFT : TW_MOUSE_RIGHT;
    TwMouseAction ma = (State == OGLDEV_KEY_STATE_PRESS) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
    
    return (TwMouseButton(ma, btn) == 1);
}


class Tutorial48 : public ICallbacks, public OgldevApp
{
public:

    Tutorial48() 
    {
        m_pGameCamera = NULL;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.66f;
        m_directionalLight.DiffuseIntensity = 1.0f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 1000.0f;  
        
        m_pipeline.SetPerspectiveProj(m_persProjInfo);           
        m_pipeline.WorldPos(Vector3f(0.0f, -8.0f, 14.0f));        
        
        m_currentMesh = BUDDHA;
    }

    ~Tutorial48()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        if (!m_atb.Init()) {
            return false;
        }
        
        Vector3f Pos(0.0f, 0.0f, 0.0f);
        Vector3f Target(0.0f, -0.1f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
              
        if (!m_LightingTech.Init()) {
            OGLDEV_ERROR("Error initializing the lighting technique\n");
            return false;
        }

        m_LightingTech.Enable();

        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetDirectionalLight(m_directionalLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);

        if (!m_mesh[BUDDHA].LoadMesh("../Content/buddha.obj")) {
            return false;            
        }
        
        if (!m_mesh[BUNNY].LoadMesh("../Content/bunny.obj")) {
            return false;            
        }

        if (!m_mesh[DRAGON].LoadMesh("../Content/dragon.obj")) {
            return false;            
        }
                
#ifndef WIN32
        // Disabled for now because it somehow clashes with the regular rendering...
 //       if (!m_fontRenderer.InitFontRenderer()) {
   //         return false;
   //     }
#endif        	    
        
        TwBar *bar;
        bar = TwNewBar("NameOfMyTweakBar");
            
        TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.

        double speed = 0.3; // Model rotation speed

        TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed, 
               " label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");

        TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, 
                  " label='Object rotation' opened=true help='Change the object orientation.' ");
        TwAddSeparator(bar, "", NULL);
        TwAddButton(bar, "Camera", NULL, NULL, "");
        TwAddVarRO(bar, "Position", TW_TYPE_DIR3F, m_pGameCamera->GetPos(), " label='Position' ");
        TwAddVarRO(bar, "Direction", TW_TYPE_DIR3F, m_pGameCamera->GetTarget(), " label='Target' ");
        TwAddSeparator(bar, "", NULL);
        TwEnumVal Meshes[] = { {BUDDHA, "Buddha"}, {BUNNY, "Bunny"}, {DRAGON, "Dragon"}};
        TwType MeshTwType = TwDefineEnum("MeshType", Meshes, 3);
        TwAddVarRW(bar, "Mesh", MeshTwType, &m_currentMesh, NULL);

        TwAddButton(bar, "Run", RunCB, NULL, " label='Run Forest' ");
        return true;
    }

    void Run()
    {
        OgldevBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        m_pGameCamera->OnRender();      

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
        m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());
        
        m_pipeline.SetCamera(*m_pGameCamera);
        float r[] = { ToDegree(g_Rotation[0]), 
                      ToDegree(g_Rotation[1]), 
                      ToDegree(g_Rotation[2]) };
        m_mesh[m_currentMesh].GetOrientation().m_rotation = Vector3f(r);
        m_pipeline.Orient(m_mesh[m_currentMesh].GetOrientation());
        m_LightingTech.SetWVP(m_pipeline.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());            
       
        m_mesh[m_currentMesh].Render();        
        	
    //    RenderFPS();     
        CalcFPS();                
        
        OgldevBackendSwapBuffers();
    }
    
       
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey)
    {
        if (!m_atb.KeyboardCB(OgldevKey)) {
            switch (OgldevKey) {
                case OGLDEV_KEY_ESCAPE:
                case OGLDEV_KEY_q:
                    OgldevBackendLeaveMainLoop();
                    break;
                default:
                    m_pGameCamera->OnKeyboard(OgldevKey);
            }
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        if (!m_atb.PassiveMouseCB(x, y)) {
            m_pGameCamera->OnMouse(x, y);
        }        
    }
    
    
    virtual void MouseCB(OGLDEV_MOUSE Button, OGLDEV_KEY_STATE State, int x, int y)
    {
        m_atb.MouseCB(Button, State, x, y);
    }

private:
        
    BasicLightingTechnique m_LightingTech;
    Camera* m_pGameCamera;
    DirectionalLight m_directionalLight;
    BasicMesh m_mesh[3];
    PersProjInfo m_persProjInfo;
    Pipeline m_pipeline;
    ATB m_atb;
    MESH_TYPE m_currentMesh;
};


int main(int argc, char** argv)
{
    OgldevBackendInit(OGLDEV_BACKEND_TYPE_GLFW, argc, argv, true, false);

    if (!OgldevBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 48")) {
        OgldevBackendTerminate();
		return 1;
    }

    SRANDOM;
    
    Tutorial48* pApp = new Tutorial48();

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
