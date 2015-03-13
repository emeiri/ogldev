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

    Tutorial 39 - Silhouette Detection
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
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "texture.h"
#include "silhouette_technique.h"
#include "ogldev_basic_lighting.h"
#include "ogldev_glut_backend.h"
#include "mesh.h"

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

class Tutorial39 : public ICallbacks, public OgldevApp
{
public:

    Tutorial39() 
    {
        m_pGameCamera = NULL;
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
        
        m_boxPos = Vector3f(0.0f, 2.0f, 0.0);
    }

    ~Tutorial39()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        Vector3f Pos(0.0f, 2.0f, -7.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
        
        if (!m_silhouetteTech.Init()) {
            printf("Error initializing the silhouette technique\n");
            return false;            
        }
      
        if (!m_LightingTech.Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }
        
        m_LightingTech.Enable();
        
        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetDirectionalLight(m_directionalLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);        

        if (!m_mesh.LoadMesh("../Content/box.obj", true)) {
            printf("Mesh load failed\n");
            return false;            
        }
        
#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif        	
        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
         

    virtual void RenderSceneCB()
    {   
        CalcFPS();
        
        m_scale += 0.01f;
               
        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                      
        RenderScene();
              
        RenderFPS();
        
        glutSwapBuffers();
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }
    

	virtual void KeyboardCB(OGLDEV_KEY OgldevKey)
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

   
private:
           
    void RenderScene()
    {
        // Render the object as-is
        m_LightingTech.Enable();
                                     
        Pipeline p;
        p.SetPerspectiveProj(m_persProjInfo);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());        
        p.WorldPos(m_boxPos);
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_LightingTech.SetWVP(p.GetWVPTrans());        
        m_mesh.Render();
        
        // Render the object's silhouette
        m_silhouetteTech.Enable();
        
        m_silhouetteTech.SetWorldMatrix(p.GetWorldTrans());        
        m_silhouetteTech.SetWVP(p.GetWVPTrans());        
        m_silhouetteTech.SetLightPos(Vector3f(0.0f, 10.0f, 0.0f));
        
        glLineWidth(5.0f);
        
        m_mesh.Render();        
    }        
    
    BasicLightingTechnique m_LightingTech;
    SilhouetteTechnique m_silhouetteTech;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
    Vector3f m_boxPos;
    Mesh m_mesh;
    PersProjInfo m_persProjInfo;
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 39")) {
        return 1;
    }

    glDepthFunc(GL_LEQUAL);
    
	SRANDOM;
    
    Tutorial39* pApp = new Tutorial39();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}