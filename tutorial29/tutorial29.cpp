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

    Tutorial 29 - 3D Picking
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <assert.h>

#include "engine_common.h"
#include "ogldev_util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "mesh.h"
#include "picking_texture.h"
#include "picking_technique.h"
#include "simple_color_technique.h"

#define WINDOW_WIDTH  1680
#define WINDOW_HEIGHT 1050

class Tutorial29 : public ICallbacks
{
public:

    Tutorial29()
    {
        m_pGameCamera = NULL;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 1.0f;
        m_directionalLight.DiffuseIntensity = 0.01f;        
        m_directionalLight.Direction = Vector3f(1.0f, -1.0, 0.0);
        m_leftMouseButton.IsPressed = false;
        m_worldPos[0] = Vector3f(-10.0f, 0.0f, 5.0f);
        m_worldPos[1] = Vector3f(10.0f, 0.0f, 5.0f);
        
        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
    }

    virtual ~Tutorial29()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
    }

    bool Init()
    {
        Vector3f Pos(0.0f, 5.0f, -22.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        if (!m_lightingEffect.Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_lightingEffect.Enable();
        m_lightingEffect.SetTextureUnit(0);
        m_lightingEffect.SetDirectionalLight(m_directionalLight);
               
        if (!m_pickingTexture.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }
        
        if (!m_pickingEffect.Init()) {
            return false;
        }
               
        if (!m_simpleColorEffect.Init()) {
            return false;
        }
        
        m_pMesh = new Mesh();

        return m_pMesh->LoadMesh("../Content/spider.obj");
    }

    void Run()
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB()
    {
        m_pGameCamera->OnRender();        

        PickingPhase();
        RenderPhase();
               
        glutSwapBuffers();
    }

    
    void PickingPhase()
    {
        Pipeline p;
        p.Scale(0.1f, 0.1f, 0.1f);
        p.Rotate(0.0f, 90.0f, 0.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);

        m_pickingTexture.EnableWriting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pickingEffect.Enable();

        for (uint i = 0 ; i < (int)ARRAY_SIZE_IN_ELEMENTS(m_worldPos) ; i++) {
            p.WorldPos(m_worldPos[i]);
            m_pickingEffect.SetObjectIndex(i);
            m_pickingEffect.SetWVP(p.GetWVPTrans());    
            m_pMesh->Render(&m_pickingEffect);
        }
        
        m_pickingTexture.DisableWriting();        
    }

    
    void RenderPhase()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Pipeline p;
        p.Scale(0.1f, 0.1f, 0.1f);
        p.Rotate(0.0f, 90.0f, 0.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        
        // If the left mouse button is clicked check if it hit a triangle
        // and color it red
        if (m_leftMouseButton.IsPressed) {
            PickingTexture::PixelInfo Pixel = m_pickingTexture.ReadPixel(m_leftMouseButton.x, WINDOW_HEIGHT - m_leftMouseButton.y - 1);
            GLExitIfError;
            if (Pixel.PrimID != 0) {                
                m_simpleColorEffect.Enable();
                assert(Pixel.ObjectID < ARRAY_SIZE_IN_ELEMENTS(m_worldPos));
                p.WorldPos(m_worldPos[Pixel.ObjectID]);
                m_simpleColorEffect.SetWVP(p.GetWVPTrans());
                // Must compensate for the decrement in the FS!
                m_pMesh->Render(Pixel.DrawID, Pixel.PrimID - 1); 
            }
        }
        
        // render the objects as usual
        m_lightingEffect.Enable();
        m_lightingEffect.SetEyeWorldPos(m_pGameCamera->GetPos());
        
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_worldPos) ; i++) {
            p.WorldPos(m_worldPos[i]);
            m_lightingEffect.SetWVP(p.GetWVPTrans());
            m_lightingEffect.SetWorldMatrix(p.GetWorldTrans());                
            m_pMesh->Render(NULL);
        }        
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
            case 27:
            case 'q':
                glutLeaveMainLoop();
                break;

            case 'a':
                m_directionalLight.AmbientIntensity += 0.05f;
                break;

            case 's':
                m_directionalLight.AmbientIntensity -= 0.05f;
                break;

            case 'z':
                m_directionalLight.DiffuseIntensity += 0.05f;
                break;

            case 'x':
                m_directionalLight.DiffuseIntensity -= 0.05f;
                break;
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }
    
    
    virtual void MouseCB(int Button, int State, int x, int y)
    {
        if (Button == GLUT_LEFT_BUTTON) {
            m_leftMouseButton.IsPressed = (State == GLUT_DOWN);
            m_leftMouseButton.x = x;
            m_leftMouseButton.y = y;
        }
    }

private:

    LightingTechnique m_lightingEffect;
    PickingTechnique m_pickingEffect;
    SimpleColorTechnique m_simpleColorEffect;
    Camera* m_pGameCamera;
    DirectionalLight m_directionalLight;
    Mesh* m_pMesh;
    PickingTexture m_pickingTexture;
    struct {
        bool IsPressed;
        int x;
        int y;
    } m_leftMouseButton;
    Vector3f m_worldPos[2];
    PersProjInfo m_persProjInfo;	
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 29")) {
        return 1;
    }

    Tutorial29* pApp = new Tutorial29();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;
 
    return 0;
}