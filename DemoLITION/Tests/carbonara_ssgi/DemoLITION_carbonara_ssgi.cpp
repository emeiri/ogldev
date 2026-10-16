/*

        Copyright 2024 Etay Meiri

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

    DemoLITION - Carbonara - SSGIDemo
*/

#include "../carbonara/carbonara.h"


class SSGIDemo : public Carbonara {

public:

    SSGIDemo()
    {
    }

    void InitChild()
    {
        SceneConfig* pConfig = m_pScene->GetConfig();

        pConfig->GetInfiniteGrid().Enabled = false;
        pConfig->ControlShadowMapping(false);
        pConfig->ControlSSGI(true);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        //m_pModel = m_pRenderingSystem->LoadModel("C:/Users/Etay Meiri/Documents/BuildArea/glTF-Sample-Assets/Models/StainedGlassLamp/glTF/StainedGlassLamp.gltf");
        m_pModel = m_pRenderingSystem->LoadModel("../Content/crytek_sponza/sponza.obj");
        //m_pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");
        //m_pModel = m_pRenderingSystem->LoadModel("../../BuildArea/glTF-Sample-Assets/Models/BoomBox/glTF/BoomBox.gltf");
        //m_pModel = m_pRenderingSystem->LoadModel("../Content/baker_and_the_bridge/scene.gltf");
       // m_pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");

        m_pTexAO = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_AO.jpg");
        m_pTexAO->Load();
        //m_pModel->GetPBRMaterial().pAO = m_pTexAO;

        m_pBRDF_LUT = new Texture(GL_TEXTURE_2D, "../Content/textures/brdfLUT.ktx");
        m_pBRDF_LUT->Load();
        pConfig->pBRDF_LUT = m_pBRDF_LUT;

        int IrrdianceMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_irradiance.ktx");
        pConfig->SetIrradianceMap(IrrdianceMap);

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");
        pConfig->SetEnvMap(EnvMap);

        int t = m_pRenderingSystem->LoadTexture2D("../Content/OpenGameArt/crate1/crate1_diffuse.png");
        m_pModel->SetColorTexture(t);

        int n = m_pRenderingSystem->LoadTexture2D("../Content/OpenGameArt/crate1/crate1_normal.png");
        m_pModel->SetNormalMap(n);

        m_pSceneObject = m_pScene->CreateSceneObject(m_pModel);
        m_pScene->AddToRenderList(m_pSceneObject);
        m_pSceneObject->SetScale(0.1f);

        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -2.5f), Vector3f(0.0f, 0.0f, 1.0f));
    }

    void OnFrameChild(double DeltaTime)
    {
        //m_pSceneObject->RotateBy(0.0f, DeltaTime / 25.0f, 0.0f);
    }

private:

    Model* m_pModel = NULL;
    SceneObject* m_pSceneObject = NULL;

    Texture* m_pTexAO = NULL;
    Texture* m_pBRDF_LUT = NULL;
};


int main(int argc, char* arg[])
{
    SSGIDemo demo;

    demo.Start();
}
