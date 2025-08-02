/*

        Copyright 2023 Etay Meiri

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
*/

#include "imgui.h"

#include "Int/core_scene.h"
#include "Int/core_rendering_system.h"

#define NUM_SCENE_OBJECTS 1024

SceneObject::SceneObject()
{
    memset(&m_rotations[0], 0, sizeof(m_rotations));
}

void SceneObject::SetRotation(const Vector3f& Rot)
{
    m_rotations[0] = Rot;
    m_numRotations = 1;
}


void SceneObject::SetRotation(float x, float y, float z)
{
    m_rotations[0].x = x;
    m_rotations[0].y = y;
    m_rotations[0].z = z;
    m_numRotations = 1;
}

void SceneObject::RotateBy(float x, float y, float z)
{
    m_rotations[0].x += x;
    m_rotations[0].y += y;
    m_rotations[0].z += z;
    m_numRotations = 1;
}


void SceneObject::PushRotation(const Vector3f& Rot)
{
    if (m_numRotations >= MAX_NUM_ROTATIONS) {
        printf("Exceeded max number of rotations - %d\n", m_numRotations);
        assert(0);
    }
        
    m_rotations[m_numRotations] = Rot;
    m_numRotations++;
}


bool QuaternionIsZero(const glm::quat& q)
{
    bool r = (q.x == 0 && q.y == 0 && q.z == 0.0f && q.w == 0.0f);
    return r;
}

Matrix4f SceneObject::GetMatrix() const
{
    Matrix4f Scale;
    Scale.InitScaleTransform(m_scale);

    Matrix4f Rotation;

    if (QuaternionIsZero(m_quaternion)) {
        CalcRotationStack(Rotation);
    } else {
        Rotation.InitRotateTransform(m_quaternion);
    }

    Matrix4f Translation;
    Translation.InitTranslationTransform(m_pos);

    Matrix4f WorldTransformation = Translation * Rotation * Scale;

    return WorldTransformation;
}


void SceneObject::CalcRotationStack(Matrix4f& Rot) const
{
    if (m_numRotations == 0) {
        Rot.InitIdentity();
    } else {
        Rot.InitRotateTransform(m_rotations[0]);

        if (m_numRotations > MAX_NUM_ROTATIONS) {
            printf("Invalid number of rotations - %d\n", m_numRotations);
            assert(0);
        }
        for (int i = 1; i < m_numRotations; i++) {
            Matrix4f r;
            r.InitRotateTransform(m_rotations[i]);
            Rot = r * Rot;
        }
    }    
}


SceneConfig::SceneConfig()
{

}



Scene::Scene()
{     
}


CoreScene::CoreScene(CoreRenderingSystem* pRenderingSystem)
{
    m_pCoreRenderingSystem = pRenderingSystem;
    CreateDefaultCamera();
    m_sceneObjects.resize(NUM_SCENE_OBJECTS);
}

void CoreScene::LoadScene(const std::string& Filename)
{
    CoreModel* pModel = (CoreModel*)m_pCoreRenderingSystem->LoadModel(Filename.c_str());
    SceneObject* pSceneObject = CreateSceneObject(pModel);
    AddToRenderList(pSceneObject);

    if (pModel->GetCameras().size() == 0) {
        printf("Warning! '%s' does not include a camera. Falling back to default.\n", Filename.c_str());
    } else {
        m_defaultCamera = pModel->GetCameras()[0];
    }    
}


void CoreScene::InitializeDefault()
{
    SceneObject* pSceneObject = CreateSceneObject("square");
    AddToRenderList(pSceneObject);
    pSceneObject->SetRotation(Vector3f(-90.0f, 0.0f, 0.0f));
    pSceneObject->SetScale(Vector3f(1000.0f, 1000.0f, 1000.0f));
    pSceneObject->SetFlatColor(Vector4f(0.5f, 0.5f, 0.5f, 1.0f));
}


void CoreScene::CreateDefaultCamera()
{
    Vector3f Pos(0.0f, 0.0f, 0.0f);
    Vector3f Target(0.0f, 0.f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;
    int WindowWidth = 0;
    int WindowHeight = 0;
    m_pCoreRenderingSystem->GetWindowSize(WindowWidth, WindowHeight);

    PersProjInfo persProjInfo = { FOV, (float)WindowWidth, (float)WindowHeight, zNear, zFar };

    Vector3f Center = Pos + Target;
    m_defaultCamera.Init(Pos.ToGLM(), Center.ToGLM(), Up.ToGLM(), persProjInfo);
}


void CoreScene::SetCamera(const Vector3f& Pos, const Vector3f& Target)
{
    m_defaultCamera.SetPos(Pos.ToGLM());
    m_defaultCamera.SetTarget(Target.ToGLM());
    m_defaultCamera.SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
}


void CoreScene::SetCameraSpeed(float Speed)
{
    printf("Warning! SetCameraSpeed is not implemented!!!\n");
 //   m_defaultCamera.SetSpeed(Speed);
}


void CoreScene::AddToRenderList(SceneObject* pSceneObject)
{
    CoreSceneObject* pCoreSceneObject = (CoreSceneObject*)pSceneObject;
    std::list<CoreSceneObject*>::const_iterator it = std::find(m_renderList.begin(), m_renderList.end(), pCoreSceneObject);

    if (it == m_renderList.end()) {
        m_renderList.push_back(pCoreSceneObject);
    }
}


bool CoreScene::RemoveFromRenderList(SceneObject* pSceneObject)
{
    std::list<CoreSceneObject*>::const_iterator it = std::find(m_renderList.begin(), m_renderList.end(), pSceneObject);

    bool ret = false;

    if (it != m_renderList.end()) {
        m_renderList.erase(it);
        ret = true;
    }

    return ret;
}



std::list<SceneObject*> CoreScene::GetSceneObjectsList()
{
    // TODO: not very efficient. Currently used only by the GUI. For small lists it should be ok.

    std::list<SceneObject*> ObjectList;

    for (std::list<CoreSceneObject*>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++) {
        ObjectList.push_back(*it);
    }

    return ObjectList;
}


void CoreScene::ShowSSAOGUI()
{
    SSAOParams& ssoaParams = m_config.GetSSAOParams();

    bool EnableSSAO = m_config.IsSSAOEnabled();
    ImGui::Checkbox("SSAO Enable", &EnableSSAO);
    m_config.ControlSSAO(EnableSSAO);
    ImGui::SliderFloat("SSAO Scale", &ssoaParams.scale, 0.0f, 2.0f);
    ImGui::SliderFloat("SSAO Bias", &ssoaParams.bias, 0.0f, 0.3f);
    ImGui::SliderFloat("SSAO Radius", &ssoaParams.radius, 0.05f, 0.5f);
    ImGui::SliderFloat("SSAO OccScale", &ssoaParams.occScale, 0.1f, 50.0f);
    ImGui::SliderFloat("SSAO AttScale", &ssoaParams.attScale, 0.5f, 1.5f);
    ImGui::SliderFloat("SSAO DistScale", &ssoaParams.distScale, 0.0f, 1.0f);    
    m_pCoreRenderingSystem->ImGuiTextureWindow("SSAO");
}


void CoreScene::ShowSceneGUI()
{
    bool EnableHDR = m_config.IsHDREnabled();
    ImGui::Checkbox("HDR Enable", &EnableHDR);
    float AverageLuminance = 0.0f;
    float Exposure = 0.0f;
    m_config.GetHDRParams(AverageLuminance, Exposure);
    ImGui::Text("HDR average luminance %.3f", AverageLuminance);
    ImGui::Text("HDR exposure %.3f", Exposure);
    m_config.ControlHDR(EnableHDR);
}


SceneObject* CoreScene::CreateSceneObject(Model* pModel)
{
    if (m_numSceneObjects == NUM_SCENE_OBJECTS) {
        printf("%s:%d - out of scene objects space\n", __FILE__, __LINE__);
        exit(0);
    }
    
    CoreSceneObject* pCoreSceneObject = CreateSceneObjectInternal((CoreModel*)pModel);

    return pCoreSceneObject;
}


SceneObject* CoreScene::CreateSceneObject(const std::string& BasicShape)
{
    CoreModel* pModel = (CoreModel*)m_pCoreRenderingSystem->GetModel(BasicShape);

    CoreSceneObject* pCoreSceneObject = CreateSceneObjectInternal(pModel);

    return pCoreSceneObject;
}


CoreSceneObject* CoreScene::CreateSceneObjectInternal(CoreModel* pModel)
{
    m_sceneObjects[m_numSceneObjects].SetModel(pModel);

    CoreSceneObject* pCoreSceneObject = &(m_sceneObjects[m_numSceneObjects]);
    int Id = m_numSceneObjects;
    pCoreSceneObject->SetId(Id);
    pCoreSceneObject->SetName("SceneObject_" + std::to_string(Id));

    m_numSceneObjects++;

    return pCoreSceneObject;
}


const std::vector<PointLight>& CoreScene::GetPointLights()
{
    if (m_pointLights.size() > 0) {
        return m_pointLights;
    }

    for (std::list<CoreSceneObject*>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++) {
        CoreSceneObject* pSceneObject = *it;

        const std::vector<PointLight>& PointLights = pSceneObject->GetModel()->GetPointLights();

        if (PointLights.size() > 0) {
            return PointLights;
        }
    }

    return m_pointLights;
}


const std::vector<SpotLight>& CoreScene::GetSpotLights()
{ 
    if (m_spotLights.size() > 0) {
        return m_spotLights;
    }

    for (std::list<CoreSceneObject*>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++) {
        CoreSceneObject* pSceneObject = *it;

        const std::vector<SpotLight>& SpotLights = pSceneObject->GetModel()->GetSpotLights();

        if (SpotLights.size() > 0) {
            return SpotLights;
        }
    }

    return m_spotLights;
}


const std::vector<DirectionalLight>& CoreScene::GetDirLights()
{ 
    if (m_dirLights.size() > 0) {
        return m_dirLights;
    }

    for (std::list<CoreSceneObject*>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++) {
        CoreSceneObject* pSceneObject = *it;

        const std::vector<DirectionalLight>& DirLights = pSceneObject->GetModel()->GetDirLights();

        if (DirLights.size() > 0) {
            return DirLights;
        }
    }

    return m_dirLights;

}

