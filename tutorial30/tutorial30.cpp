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

    Tutorial 30 - Basic Tessellation
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "lighting_technique.h"
#include "ogldev_glut_backend.h"
#include "mesh.h"

#define WINDOW_WIDTH  1680
#define WINDOW_HEIGHT 1050

class Tutorial30 : public ICallbacks, public OgldevApp
{
public:

    Tutorial30()
    {
        m_pGameCamera = NULL;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 1.0f;
        m_directionalLight.DiffuseIntensity = 0.01f;        
        m_directionalLight.Direction = Vector3f(1.0f, -1.0, 0.0);
        
        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_pDisplacementMap = NULL;
        m_dispFactor = 0.25f;
        m_isWireframe = false;
    }

    virtual ~Tutorial30()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
        SAFE_DELETE(m_pDisplacementMap);
    }

    bool Init()
    {
        Vector3f Pos(0.0f, 1.0f, -5.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);       

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        if (!m_lightingEffect.Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }
               
        GLint MaxPatchVertices = 0;
        glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
        printf("Max supported patch vertices %d\n", MaxPatchVertices);
        //glPatchParameteri(GL_PATCH_VERTICES, 3);
     
        glActiveTexture(GL_TEXTURE4);
        m_pDisplacementMap = new Texture(GL_TEXTURE_2D, "../Content/heightmap.jpg");
        
        if (!m_pDisplacementMap->Load()) {
            return false;
        }
        
        m_pDisplacementMap->Bind(DISPLACEMENT_TEXTURE_UNIT);
                
        glActiveTexture(GL_TEXTURE0);
        m_pColorMap = new Texture(GL_TEXTURE_2D, "../Content/diffuse.jpg");
        
        if (!m_pColorMap->Load()) {
            return false;
        }
        
        m_pColorMap->Bind(COLOR_TEXTURE_UNIT);

        m_lightingEffect.Enable();

        m_lightingEffect.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_lightingEffect.SetDisplacementMapTextureUnit(DISPLACEMENT_TEXTURE_UNIT_INDEX);
        m_lightingEffect.SetDirectionalLight(m_directionalLight);
        m_lightingEffect.SetDispFactor(m_dispFactor);                        
        m_pMesh = new Mesh();

        return m_pMesh->LoadMesh("../Content/quad2.obj");
    }

    void Run()
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_pGameCamera->OnRender();        

        Pipeline p;
        p.Scale(2.0f, 2.0f, 2.0f);               
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        
        // render the objects as usual
        m_lightingEffect.Enable();

        m_lightingEffect.SetEyeWorldPos(m_pGameCamera->GetPos());        
        m_lightingEffect.SetVP(p.GetVPTrans());
        m_lightingEffect.SetWorldMatrix(p.GetWorldTrans());                
        m_lightingEffect.SetDispFactor(m_dispFactor);

        m_pMesh->Render(NULL);

        glutSwapBuffers();
    }

       
	virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
    {
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
			break;

            case OGLDEV_KEY_PLUS:
                m_dispFactor += 0.01f;
                break;
                
            case OGLDEV_KEY_MINUS:
                if (m_dispFactor >= 0.01f) {
                    m_dispFactor -= 0.01f;                    
                }
                break;
                
            case 'z':
                m_isWireframe = !m_isWireframe;
                
                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT, GL_FILL);
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

    LightingTechnique m_lightingEffect;
    Camera* m_pGameCamera;
    DirectionalLight m_directionalLight;
    Mesh* m_pMesh;
    PersProjInfo m_persProjInfo;
    Texture* m_pDisplacementMap;
    Texture* m_pColorMap;
    float m_dispFactor;
    bool m_isWireframe;
};


int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 30")) {
        return 1;
    }

    Tutorial30* pApp = new Tutorial30();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;
 
    return 0;
}