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

    Tutorial 37 - Deferred Shading - Part 3
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "engine_common.h"
#include "ogldev_util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "ds_geom_pass_tech.h"
#include "ds_point_light_pass_tech.h"
#include "ds_dir_light_pass_tech.h"
#include "null_technique.h"
#include "glut_backend.h"
#include "mesh.h"
#ifndef WIN32
#include "freetypeGL.h"
#endif
#include "gbuffer.h"
#include "lights_common.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

class Tutorial37 : public ICallbacks
{
public:

    Tutorial37() 
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_frameCount = 0;
        m_fps = 0.0f;
		
        InitLights();
        InitBoxPositions();
        
        m_time = glutGet(GLUT_ELAPSED_TIME);
    }


    ~Tutorial37()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        if (!m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_DSGeomPassTech.Init()) {
            printf("Error initializing DSGeomPassTech\n");
            return false;
        }      

		m_DSGeomPassTech.Enable();
		m_DSGeomPassTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

		if (!m_DSPointLightPassTech.Init()) {
			printf("Error initializing DSPointLightPassTech\n");
			return false;
		}
	
		m_DSPointLightPassTech.Enable();

		m_DSPointLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
		m_DSPointLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		m_DSPointLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        m_DSPointLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);

		if (!m_DSDirLightPassTech.Init()) {
			printf("Error initializing DSDirLightPassTech\n");
			return false;
		}
	
		m_DSDirLightPassTech.Enable();

		m_DSDirLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
		m_DSDirLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		m_DSDirLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		m_DSDirLightPassTech.SetDirectionalLight(m_dirLight);
        m_DSDirLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        Matrix4f WVP;
        WVP.InitIdentity();
        m_DSDirLightPassTech.SetWVP(WVP);

		if (!m_nullTech.Init()) {
			return false;
		}
        
        if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }

        if (!m_box.LoadMesh("../Content/box.obj")) {
			return false;
		}

        if (!m_bsphere.LoadMesh("../Content/sphere.obj")) {
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
        
        m_scale += 0.05f;

        m_pGameCamera->OnRender();

		m_gbuffer.StartFrame();

        DSGeometryPass();

		// We need stencil to be enabled in the stencil pass to get the stencil buffer
		// updated and we also need it in the light pass because we render the light
		// only if the stencil passes.
		glEnable(GL_STENCIL_TEST);

		for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLight); i++) {
			DSStencilPass(i);
			DSPointLightPass(i);
		}

		// The directional light does not need a stencil test because its volume
		// is unlimited and the final pass simply copies the texture.
		glDisable(GL_STENCIL_TEST);

		DSDirectionalLightPass();

		DSFinalPass();
        
        RenderFPS();
        
        glutSwapBuffers();
    }
    

    void DSGeometryPass()
    {
		m_DSGeomPassTech.Enable();

        m_gbuffer.BindForGeomPass();

		// Only the geometry pass updates the depth buffer
		glDepthMask(GL_TRUE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);        
        p.Rotate(0.0f, m_scale, 0.0f);
        
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boxPositions) ; i++) {
            p.WorldPos(m_boxPositions[i]);
            m_DSGeomPassTech.SetWVP(p.GetWVPTrans());
        	m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());
            m_box.Render();            
        }

		// When we get here the depth buffer is already populated and the stencil pass
		// depends on it, but it does not write to it.
		glDepthMask(GL_FALSE);		
    }

	void DSStencilPass(unsigned int PointLightIndex)
	{
		m_nullTech.Enable();

		// Disable color/depth write and enable stencil
		m_gbuffer.BindForStencilPass();
		glEnable(GL_DEPTH_TEST);

        glDisable(GL_CULL_FACE);

		glClear(GL_STENCIL_BUFFER_BIT);

		// We need the stencil test to be enabled but we want it
		// to succeed always. Only the depth test matters.
		glStencilFunc(GL_ALWAYS, 0, 0);

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		Pipeline p;
		p.WorldPos(m_pointLight[PointLightIndex].Position);
        float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);
		p.Scale(BBoxScale, BBoxScale, BBoxScale);		
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);

		m_nullTech.SetWVP(p.GetWVPTrans());
		m_bsphere.Render();  
	}

    
    void DSPointLightPass(unsigned int PointLightIndex)
    {
		m_gbuffer.BindForLightPass();

        m_DSPointLightPassTech.Enable();
        m_DSPointLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());        

		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
      		
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        Pipeline p;
        p.WorldPos(m_pointLight[PointLightIndex].Position);
        float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);        
		p.Scale(BBoxScale, BBoxScale, BBoxScale);		
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);               
        m_DSPointLightPassTech.SetWVP(p.GetWVPTrans());
        m_DSPointLightPassTech.SetPointLight(m_pointLight[PointLightIndex]);
        m_bsphere.Render(); 
        glCullFace(GL_BACK);
       
		glDisable(GL_BLEND);
    }
	

	void DSDirectionalLightPass()
	{
		m_gbuffer.BindForLightPass();

        m_DSDirLightPassTech.Enable();
        m_DSDirLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

        m_quad.Render();  

		glDisable(GL_BLEND);
	}


	void DSFinalPass()
	{
		m_gbuffer.BindForFinalPass();
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 
                          0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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

    // The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation)
    float CalcPointLightBSphere(const PointLight& Light)
    {
        float MaxChannel = fmax(fmax(Light.Color.x, Light.Color.y), Light.Color.z);
        
        float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear - 4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - 256 * MaxChannel * Light.DiffuseIntensity))) 
                    /
                    2 * Light.Attenuation.Exp;
        
        return ret;
    }    
        
    void InitLights()
    {
        m_spotLight.AmbientIntensity = 0.0f;
        m_spotLight.DiffuseIntensity = 0.9f;
		m_spotLight.Color = COLOR_WHITE;
        m_spotLight.Attenuation.Linear = 0.01f;
        m_spotLight.Position  = Vector3f(-20.0, 20.0, 5.0f);
        m_spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.Cutoff =  20.0f;

		m_dirLight.AmbientIntensity = 0.1f;
		m_dirLight.Color = COLOR_CYAN;
		m_dirLight.DiffuseIntensity = 0.5f;
		m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);

		m_pointLight[0].DiffuseIntensity = 0.2f;
		m_pointLight[0].Color = COLOR_GREEN;
        m_pointLight[0].Position = Vector3f(0.0f, 1.5f, 5.0f);
		m_pointLight[0].Attenuation.Constant = 0.0f;
        m_pointLight[0].Attenuation.Linear = 0.0f;
        m_pointLight[0].Attenuation.Exp = 0.3f;

		m_pointLight[1].DiffuseIntensity = 0.2f;
		m_pointLight[1].Color = COLOR_RED;
        m_pointLight[1].Position = Vector3f(2.0f, 0.0f, 5.0f);
		m_pointLight[1].Attenuation.Constant = 0.0f;
        m_pointLight[1].Attenuation.Linear = 0.0f;
        m_pointLight[1].Attenuation.Exp = 0.3f;
        
		m_pointLight[2].DiffuseIntensity = 0.2f;
		m_pointLight[2].Color = COLOR_BLUE;
        m_pointLight[2].Position = Vector3f(0.0f, 0.0f, 3.0f);
		m_pointLight[2].Attenuation.Constant = 0.0f;
        m_pointLight[2].Attenuation.Linear = 0.0f;        
        m_pointLight[2].Attenuation.Exp = 0.3f;
    }
    
      
    void InitBoxPositions()
    {
        m_boxPositions[0] = Vector3f(0.0f, 0.0f, 5.0f);
        m_boxPositions[1] = Vector3f(6.0f, 1.0f, 10.0f);
        m_boxPositions[2] = Vector3f(-5.0f, -1.0f, 12.0f);
        m_boxPositions[3] = Vector3f(4.0f, 4.0f, 15.0f);
        m_boxPositions[4] = Vector3f(-4.0f, 2.0f, 20.0f);
    }
    
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

	DSGeomPassTech m_DSGeomPassTech;
	DSPointLightPassTech m_DSPointLightPassTech;
    DSDirLightPassTech m_DSDirLightPassTech;
    NullTechnique m_nullTech;
    Camera* m_pGameCamera;
    float m_scale;
    SpotLight m_spotLight;
	DirectionalLight m_dirLight;
	PointLight m_pointLight[3];
    Mesh m_box;
    Mesh m_bsphere;
    Mesh m_quad;
    PersProjInfo m_persProjInfo;
#ifndef WIN32
    FontRenderer m_fontRenderer;
#endif
    int m_time;
    int m_frameCount;
    float m_fps;    
    GBuffer m_gbuffer;
    Vector3f m_boxPositions[5];
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 37")) {
        return 1;
    }
    
    Tutorial37* pApp = new Tutorial37();

    if (!pApp->Init()) {
        return 1;
    }       
    
    pApp->Run();

    delete pApp;
 
    return 0;
}