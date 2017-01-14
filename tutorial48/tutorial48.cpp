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


    Tutorial 48 - User Interface with Ant Tweak Bar
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
#include "ogldev_basic_lighting.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_atb.h"

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

Quaternion g_Rotation = Quaternion(0.707f, 0.0f, 0.0f, 0.707f);

typedef enum { BUDDHA, BUNNY, DRAGON } MESH_TYPE;

bool gAutoRotate = false;
int gGLMajorVersion = 0;

void TW_CALL AutoRotateCB(void *p)
{ 
    gAutoRotate = !gAutoRotate;
}


class Tutorial48 : public ICallbacks, public OgldevApp
{
public:

    Tutorial48() 
    {
        m_pGameCamera = NULL;
        
        m_directionalLight.Name = "DirLight1";
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.66f;
        m_directionalLight.DiffuseIntensity = 1.0f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

        m_persProjInfo.FOV    = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width  = WINDOW_WIDTH;
        m_persProjInfo.zNear  = 1.0f;
        m_persProjInfo.zFar   = 1000.0f;  
        
        m_pipeline.SetPerspectiveProj(m_persProjInfo);                   
        
        m_currentMesh = BUDDHA;
        m_rotationSpeed = 0.2f;
        
        glGetIntegerv(GL_MAJOR_VERSION, &gGLMajorVersion);
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
            OGLDEV_ERROR0("Error initializing the lighting technique\n");
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
        m_mesh[BUDDHA].GetOrientation().m_rotation.y = 180.0f;
        
        if (!m_mesh[BUNNY].LoadMesh("../Content/bunny.obj")) {
            return false;            
        }

        if (!m_mesh[DRAGON].LoadMesh("../Content/dragon.obj")) {
            return false;            
        }
         
        for (int i = 0 ; i < 3 ; i++) {
            m_mesh[i].GetOrientation().m_pos = Vector3f(0.0f, -8.0f, 34.0f);
        }
#ifndef WIN32
        // Disabled for now because it somehow clashes with the regular rendering...
 //       if (!m_fontRenderer.InitFontRenderer()) {
   //         return false;
   //     }
#endif        	    
        bar = TwNewBar("OGLDEV");

        TwEnumVal Meshes[] = { {BUDDHA, "Buddha"}, {BUNNY, "Bunny"}, {DRAGON, "Dragon"}};
        TwType MeshTwType = TwDefineEnum("MeshType", Meshes, 3);
        TwAddVarRW(bar, "Mesh", MeshTwType, &m_currentMesh, NULL);
        
        TwAddSeparator(bar, "", NULL);
        
        m_pGameCamera->AddToATB(bar);
        
        TwAddSeparator(bar, "", NULL);
        
        TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, " axisz=-z ");
        
        TwAddButton(bar, "AutoRotate", AutoRotateCB, NULL, " label='Auto rotate' ");
        
        TwAddVarRW(bar, "Rot Speed", TW_TYPE_FLOAT, &m_rotationSpeed, 
                   " min=0 max=5 step=0.1 keyIncr=s keyDecr=d help='Rotation speed (turns/second)' ");

        TwAddSeparator(bar, "", NULL);
        
        m_directionalLight.AddToATB(bar);
        
        float refresh = 0.1f;
        TwSetParam(bar, NULL, "refresh", TW_PARAM_FLOAT, 1, &refresh);                
            
        TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with OGLDEV.' "); // Message added to the help bar.
                                                        
        TwAddVarRO(bar, "GL Major Version", TW_TYPE_INT32, &gGLMajorVersion, " label='Major version of GL' ");
        
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
        m_LightingTech.SetDirectionalLight(m_directionalLight);
        
        m_pipeline.SetCamera(*m_pGameCamera);

        if (gAutoRotate) {
            m_mesh[m_currentMesh].GetOrientation().m_rotation.y += m_rotationSpeed;
        }
        else {
            m_mesh[m_currentMesh].GetOrientation().m_rotation = g_Rotation.ToDegrees();
        }
        
        m_pipeline.Orient(m_mesh[m_currentMesh].GetOrientation());
        m_LightingTech.SetWVP(m_pipeline.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(m_pipeline.GetWorldTrans());            
       
        m_mesh[m_currentMesh].Render();        
        	
      //  RenderFPS();     
        CalcFPS();                
        
        OgldevBackendSwapBuffers();
    }
    
       
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE OgldevKeyState)
    {
        if (OgldevKeyState == OGLDEV_KEY_STATE_PRESS) {
            if (m_atb.KeyboardCB(OgldevKey)) {
                return;
            }
        }
        
        switch (OgldevKey) {
            case OGLDEV_KEY_A:
            {
                int Pos[2], Size[2];
                TwGetParam(bar, NULL, "position", TW_PARAM_INT32, 2, Pos);
                TwGetParam(bar, NULL, "size", TW_PARAM_INT32, 2, Size);
                OgldevBackendSetMousePos(Pos[0] + Size[0]/2, 
                                         Pos[1] + Size[1]/2);
                break;
            }
            case OGLDEV_KEY_B:
                m_currentMesh = DRAGON;
                break;
            case OGLDEV_KEY_C:
                m_currentMesh = BUDDHA;
                break;                    
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
    float m_rotationSpeed;
    TwBar *bar;
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
