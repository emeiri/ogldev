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

    Tutorial 33 - Instanced Rendering
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

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

#define NUM_ROWS 50
#define NUM_COLS 20
#define NUM_INSTANCES NUM_ROWS * NUM_COLS


class Tutorial33 : public ICallbacks, public OgldevApp
{
public:

    Tutorial33()
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
        
        m_pMesh = NULL;
    }

    ~Tutorial33()
    {
        SAFE_DELETE(m_pEffect);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
    }    

    bool Init()
    {
        Vector3f Pos(7.0f, 3.0f, 0.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        m_pEffect = new LightingTechnique();

        if (!m_pEffect->Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_pEffect->Enable();

        m_pEffect->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pEffect->SetDirectionalLight(m_directionalLight);
        m_pEffect->SetMatSpecularIntensity(0.0f);
        m_pEffect->SetMatSpecularPower(0);
        m_pEffect->SetColor(0, Vector4f(1.0f, 0.5f, 0.5f, 0.0f));
        m_pEffect->SetColor(1, Vector4f(0.5f, 1.0f, 1.0f, 0.0f));
        m_pEffect->SetColor(2, Vector4f(1.0f, 0.5f, 1.0f, 0.0f));
        m_pEffect->SetColor(3, Vector4f(1.0f, 1.0f, 1.0f, 0.0f));

        m_pMesh = new Mesh();

        if (!m_pMesh->LoadMesh("../Content/spider.obj")) {
            return false;            
        }
        
#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif
        
        CalcPositions();
        
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
        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
        
        Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);   
        p.Rotate(0.0f, 90.0f, 0.0f);
        p.Scale(0.005f, 0.005f, 0.005f);                

        Matrix4f WVPMatrics[NUM_INSTANCES];
        Matrix4f WorldMatrices[NUM_INSTANCES];
        
        for (unsigned int i = 0 ; i < NUM_INSTANCES ; i++) {
            Vector3f Pos(m_positions[i]);
            Pos.y += sinf(m_scale) * m_velocity[i];
            p.WorldPos(Pos);        
            WVPMatrics[i] = p.GetWVPTrans().Transpose();
            WorldMatrices[i] = p.GetWorldTrans().Transpose();
        }
        
        m_pMesh->Render(NUM_INSTANCES, WVPMatrics, WorldMatrices);
        
        RenderFPS();
        
        glutSwapBuffers();
    }


	void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
	{
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
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
    
    void CalcPositions()
    {
        for (unsigned int i = 0; i < NUM_ROWS ; i++) {
            for (unsigned int j = 0 ; j < NUM_COLS ; j++) {
                unsigned int Index = i * NUM_COLS + j;
                m_positions[Index].x = (float)j;
                m_positions[Index].y = RandomFloat() * 5.0f;
                m_positions[Index].z = (float)i;
                m_velocity[Index] = RandomFloat();
                if (i & 1) {
                    m_velocity[Index] *= (-1.0f);
                }
            }
        }                   
    }

    LightingTechnique* m_pEffect;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
    Mesh* m_pMesh;
    PersProjInfo m_persProjInfo;
    Vector3f m_positions[NUM_INSTANCES];            
    float m_velocity[NUM_INSTANCES];
};


int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 33")) {
        return 1;
    }
    
    SRANDOM;

    Tutorial33* pApp = new Tutorial33();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}