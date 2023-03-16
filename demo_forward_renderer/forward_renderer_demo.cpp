/*

        Copyright 2021 Etay Meiri

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

    Forward Renderer Demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_world_transform.h"
#include "ogldev_rendering_subsystem.h"
#include "ogldev_skinned_mesh.h"
#include "ogldev_forward_lighting.h"
#include "ogldev_forward_renderer.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


class ForwardRendererDemo : public GameCallbacks
{
public:

    ForwardRendererDemo()
    {
        m_pointLights[0].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        m_pointLights[0].DiffuseIntensity = 1.0f;
        m_pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[0].Attenuation.Linear = 0.1f;
        m_pointLights[0].Attenuation.Exp = 0.0f;

        m_pointLights[1].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        m_pointLights[1].DiffuseIntensity = 0.25f;
        m_pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[1].Attenuation.Linear = 0.0f;
        m_pointLights[1].Attenuation.Exp = 0.2f;

        m_spotLights[0].DiffuseIntensity = 1.0f;
        m_spotLights[0].Color = Vector3f(1.0f, 0.0f, 0.0f);
        m_spotLights[0].Attenuation.Linear = 0.1f;
        m_spotLights[0].Cutoff = 5.0f;
        m_spotLights[0].WorldDirection = Vector3f(0.0f, 0.0f, 1.0f);

        m_spotLights[1].DiffuseIntensity = 0.0f;
        m_spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_spotLights[1].Attenuation.Linear = 0.01f;
        m_spotLights[1].Cutoff = 30.0f;
        m_spotLights[1].WorldPosition = Vector3f(0.0f, 1.0f, 0.0f);
        m_spotLights[1].WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);

        m_dirLight.WorldDirection = Vector3f(0.0f, 0.0f, 1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
    }

    virtual ~ForwardRendererDemo()
    {
        SAFE_DELETE(m_pMesh);
        SAFE_DELETE(m_pMesh1);
    }


    void Init()
    {
        m_pRenderingSubsystem = BaseRenderingSubsystem::CreateRenderingSubsystem(RENDERING_SUBSYSTEM_GL, this);

        m_pRenderingSubsystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

        InitMesh();

        InitRenderer();

        m_startTimeMillis = GetCurrentTimeMillis();
    }


    void Run()
    {
        m_pRenderingSubsystem->Execute();
    }

    void OnFrame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef _WIN64
        float YRotationAngle = 0.1f;
#else
        float YRotationAngle = 1.0f;
#endif
        m_counter += 0.1f;

        /*        m_pointLights[1].WorldPosition.y = sinf(m_counter) * 4 + 4;
        m_renderer.UpdatePointLightPos(1, m_pointLights[1].WorldPosition);

        m_spotLights[0].WorldPosition = m_pGameCamera->GetPos();
        m_spotLights[0].WorldDirection = m_pGameCamera->GetTarget();
        m_renderer.UpdateSpotLightPosAndDir(0, m_spotLights[0].WorldPosition, m_spotLights[0].WorldDirection);
        */
        long long CurrentTimeMillis = GetCurrentTimeMillis();
        float AnimationTimeSec = ((float)(CurrentTimeMillis - m_startTimeMillis)) / 1000.0f;

        m_pMesh->SetRotation(0.0f, m_counter, 0.0f);
        m_renderer.Render(m_pMesh);

        //m_renderer.RenderAnimation(m_pMesh1, AnimationTimeSec);
    }


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

    bool OnKeyboard(int key, int state)
    {
        bool Handled = true;

        switch (key) {

        case 'a':
            m_pointLights[0].Attenuation.Linear += ATTEN_STEP;
            m_pointLights[1].Attenuation.Linear += ATTEN_STEP;
            break;

        case 'z':
            m_pointLights[0].Attenuation.Linear -= ATTEN_STEP;
            m_pointLights[1].Attenuation.Linear -= ATTEN_STEP;
            break;

        case 's':
            m_pointLights[0].Attenuation.Exp += ATTEN_STEP;
            m_pointLights[1].Attenuation.Exp += ATTEN_STEP;
            break;

        case 'x':
            m_pointLights[0].Attenuation.Exp -= ATTEN_STEP;
            m_pointLights[1].Attenuation.Exp -= ATTEN_STEP;
            break;

        case 'd':
            m_spotLights[0].Cutoff += ANGLE_STEP;
            break;

        case 'c':
            m_spotLights[0].Cutoff -= ANGLE_STEP;
            break;

        case 'g':
            m_spotLights[1].Cutoff += ANGLE_STEP;
            break;

        case 'b':
            m_spotLights[1].Cutoff -= ANGLE_STEP;
            break;

        default:
            Handled = false;
        }

        return Handled;
    }


private:

    void InitRenderer()
    {
        m_renderer.InitForwardRenderer(m_pRenderingSubsystem);
        //        m_renderer.SetPointLights(2, m_pointLights);
        //        m_renderer.SetSpotLights(2, m_spotLights);
        m_renderer.SetDirLight(m_dirLight);
    }


    void InitMesh()
    {
        m_pMesh = new BasicMesh();
        m_pMesh->LoadMesh("../Content/sphere.obj");
        m_pMesh->SetPosition(0.0f, 0.0f, 10.0f);

        m_pMesh1 = new SkinnedMesh();
        m_pMesh1->LoadMesh("../Content/iclone-7-raptoid-mascot/scene.gltf");
        m_pMesh1->SetPosition(0.0f, 0.0f, 15.0f);
        m_pMesh1->SetRotation(90.0f, 0.0f, 0.0f);
        m_pMesh1->SetScale(0.05f);
    }

    BasicMesh* m_pMesh = NULL;
    SkinnedMesh* m_pMesh1 = NULL;
    PersProjInfo m_persProjInfo;
    PointLight m_pointLights[ForwardLightingTechnique::MAX_POINT_LIGHTS];
    SpotLight m_spotLights[ForwardLightingTechnique::MAX_SPOT_LIGHTS];
    DirectionalLight m_dirLight;
    float m_counter = 0;
    long long m_startTimeMillis = 0;
    BaseRenderingSubsystem* m_pRenderingSubsystem = NULL;
    ForwardRenderer m_renderer;
};


int main(int argc, char** argv)
{
    ForwardRendererDemo App;
    App.Init();
    App.Run();

    return 0;
}
