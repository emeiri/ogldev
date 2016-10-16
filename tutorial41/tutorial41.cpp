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

    Tutorial 41 - Motion Blur
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

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "skinning_technique.h"
#include "motion_blur_technique.h"
#include "ogldev_glut_backend.h"
#include "ogldev_skinned_mesh.h"
#include "intermediate_buffer.h"

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024

class Tutorial41 : public ICallbacks, public OgldevApp
{
public:

    Tutorial41() 
    {
        m_pGameCamera = NULL;
        m_pSkinningTech = NULL;
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
        
        m_position = Vector3f(0.0f, 0.0f, 6.0f);      
    }

    ~Tutorial41()
    {
        SAFE_DELETE(m_pSkinningTech);
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
      
        m_pSkinningTech = new SkinningTechnique();

        if (!m_pSkinningTech->Init()) {
            printf("Error initializing the skinning technique\n");
            return false;
        }

        m_pSkinningTech->Enable();

        m_pSkinningTech->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pSkinningTech->SetDirectionalLight(m_directionalLight);
        m_pSkinningTech->SetMatSpecularIntensity(0.0f);
        m_pSkinningTech->SetMatSpecularPower(0);

        m_pMotionBlurTech = new MotionBlurTechnique();

        if (!m_pMotionBlurTech->Init()) {
            printf("Error initializing the motion blur technique\n");
            return false;
        }
        
        m_pMotionBlurTech->Enable();
        m_pMotionBlurTech->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_pMotionBlurTech->SetMotionTextureUnit(MOTION_TEXTURE_UNIT_INDEX);

        if (!m_mesh.LoadMesh("../Content/boblampclean.md5mesh")) {
            printf("Mesh load failed\n");
            return false;            
        }
        
        m_mesh.BoneTransform(0.0f, m_prevTransforms);

        if (!m_quad.LoadMesh("../Content/quad_r.obj")) {
            printf("Quad mesh load failed\n");
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
              
        m_pGameCamera->OnRender();

        RenderPass();
        
        MotionBlurPass();
                              
        RenderFPS();       
        
        glutSwapBuffers();
    }
    
    void RenderPass()
    {
        m_intermediateBuffer.BindForWriting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pSkinningTech->Enable();
        
        vector<Matrix4f> Transforms;
               
        float RunningTime = GetRunningTime();

        m_mesh.BoneTransform(RunningTime, Transforms);
        
        for (uint i = 0 ; i < Transforms.size() ; i++) {
            m_pSkinningTech->SetBoneTransform(i, Transforms[i]);
            m_pSkinningTech->SetPrevBoneTransform(i, m_prevTransforms[i]);
        }                
               
        m_pSkinningTech->SetEyeWorldPos(m_pGameCamera->GetPos());
        
        m_pipeline.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        m_pipeline.SetPerspectiveProj(m_persProjInfo);           
        m_pipeline.Scale(0.1f, 0.1f, 0.1f);                
             
        Vector3f Pos(m_position);
        m_pipeline.WorldPos(Pos);        
        m_pipeline.Rotate(270.0f, 180.0f, 0.0f);       
        m_pSkinningTech->SetWVP(m_pipeline.GetWVPTrans());
        m_pSkinningTech->SetWorldMatrix(m_pipeline.GetWorldTrans());            
       
        m_mesh.Render();        
        
        m_prevTransforms = Transforms;
    }
    
    
    void MotionBlurPass()
    {
        m_intermediateBuffer.BindForReading();

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        m_pMotionBlurTech->Enable();
       
        m_quad.Render();
    }


    virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
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
        
    SkinningTechnique* m_pSkinningTech;
    MotionBlurTechnique* m_pMotionBlurTech;
    Camera* m_pGameCamera;
    DirectionalLight m_directionalLight;
    SkinnedMesh m_mesh;
    SkinnedMesh m_quad;
    Vector3f m_position;            
    PersProjInfo m_persProjInfo;
    IntermediateBuffer m_intermediateBuffer;
    Pipeline m_pipeline;
    vector<Matrix4f> m_prevTransforms;
};


int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 41")) {
        return 1;
    }
    
    SRANDOM;
    
    Tutorial41* pApp = new Tutorial41();

    if (!pApp->Init()) {
        return 1;
    }
    
    pApp->Run();

    delete pApp;
 
    return 0;
}